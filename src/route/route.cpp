#include <string>

#include "route.h"
#include "../entity/employee.h"

using namespace std;

void register_api_routes(CrowApp& app) {
    
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([](){

        //-----------------
        // Return a string

        // return "Hello world";

        //-------------------
        // Return a template
        
        auto page = crow::mustache::load_text("testing/hello_world.html");
        return page;
    });

    CROW_ROUTE(app, "/hello/<string>").methods(crow::HTTPMethod::GET)([](string name){

        CROW_LOG_WARNING << "Hello, " << name;

        //----------------------------------
        // Return a template with variables

        auto page = crow::mustache::load("testing/hello_person.html");
        crow::mustache::context ctx ({{"person", name}});
        return page.render(ctx);
    });

    CROW_ROUTE(app, "/request/details").methods(crow::HTTPMethod::GET)([](const crow::request& req){

        // https://crowcpp.org/master/reference/structcrow_1_1request.html
        // https://crowcpp.org/master/reference/structcrow_1_1response.html
        
        // 1. Check if a specific query parameter exists
        if (req.url_params.get("asset") != nullptr) {
            // 2. Safely extract it as a string
            std::string asset = req.url_params.get("asset");
            return crow::response(crow::status::OK, "You requested data for: " + asset);
        }
        
        // Fallback to returning the raw URL if no parameters were passed
        return crow::response(crow::status::OK, req.raw_url);
    });

    CROW_ROUTE(app, "/employee/<string>").methods(crow::HTTPMethod::GET)([](string name){

        //---------------------------
        // Return a returnable class

        Employee employee {name};
        return employee;
    });
    
    //---------------------------------------------------------------
    // Endpoint that accepts a JSON configuration for a trading pair

    // Testing with:
    //   curl -k -X POST https://localhost:18080/api/config -H "Content-Type: application/json" -d '{"ticker": "BTC_USD", "leverage": 10, "indicators": ["MA", "RSI"]}'

    CROW_ROUTE(app, "/api/config").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        
        //---------------------------------------
        // 1. USING rvalue TO READ INCOMING JSON
        
        // Parse the raw request body text into a Read-Value (rvalue)
        crow::json::rvalue incoming_json = crow::json::load(req.body);

        // Safety check: Validate that the input text was actually valid JSON
        if (!incoming_json) {
            return crow::response(400, "Invalid JSON payload received.");
        }

        // Validate that required keys exist before reading them
        if (!incoming_json.has("ticker") || !incoming_json.has("leverage")) {
            return crow::response(400, "Missing required fields: 'ticker' or 'leverage'.");
        }

        // Extract values using standard primitive types (.s() for string, .i() for int, .d() for double)
        std::string ticker = incoming_json["ticker"].s();
        int leverage = incoming_json["leverage"].i();
        
        // Optional reading: nested array parsing if it exists
        bool has_indicators = incoming_json.has("indicators");

        std::cout << "Processing config for " << ticker << " with " << leverage << "x leverage.\n";

        //----------------------------------------
        // 2. USING wvalue TO WRITE OUTGOING JSON
        
        // Create a Write-Value (wvalue) to construct our response back to the client
        crow::json::wvalue outgoing_json;

        if (leverage > 100) {
            // Build an error payload
            outgoing_json["status"] = "rejected";
            outgoing_json["reason"] = "Leverage exceeds risk limits.";
            return crow::response(400, outgoing_json);
        }

        // Build a success nested structure manually
        outgoing_json["status"] = "success";
        outgoing_json["received"]["ticker"] = ticker;
        outgoing_json["received"]["leverage"] = leverage;
        
        // Dynamically append lists or arrays to a wvalue
        if (has_indicators) {
            int index = 0;
            for (const auto& indicator : incoming_json["indicators"]) {
                outgoing_json["applied_indicators"][index++] = indicator.s();
            }
        } else {
            outgoing_json["applied_indicators"] = crow::json::wvalue::list(); // Empty JSON array []
        }

        // Crow automatically converts a crow::json::wvalue parameter inside 
        // a response object into a formatted JSON string and sets application/json headers.
        return crow::response(200, outgoing_json);
    });

    //-----------------------------------------------------------
    // Define a POST endpoint for handling file and form uploads

    // Testing with:
    //   echo "Date,Close\n2026-06-01,150.5\n2026-06-02,152.3" > data.csv
    //   curl -k -X POST https://localhost:18080/api/upload -F "asset=AUD_USD" -F "timeframe=1d" -F "csv_file=@data.csv"

    CROW_ROUTE(app, "/api/upload").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        
        // 1. Parse the incoming request into a multipart message
        crow::multipart::message mp_msg(req);

        // 2. Extract plain text form fields using `get_part_by_name()`
        //    Let's assume the form passes an "asset" and a "timeframe" text field
        auto asset_part     = mp_msg.get_part_by_name("asset");
        auto timeframe_part = mp_msg.get_part_by_name("timeframe");

        if (asset_part.body.empty() || timeframe_part.body.empty()) {
            return crow::response(400, "Missing required form fields 'asset' or 'timeframe'.");
        }

        std::string asset     = asset_part.body;
        std::string timeframe = timeframe_part.body;
        
        std::cout << "Received metadata for asset: " << asset << " (" << timeframe << ")\n";

        // 3. Extract the uploaded file part (let's assume the input field name is "csv_file")
        auto file_part = mp_msg.get_part_by_name("csv_file");
        if (file_part.body.empty()) {
            return crow::response(400, "Missing uploaded file 'csv_file'.");
        }

        // 4. Retrieve metadata from the file's part headers (like the actual filename)
        std::string filename = "uploaded_data.csv"; // Fallback name
        
        auto headers_it = file_part.headers.find("Content-Disposition");

        if (headers_it != file_part.headers.end()) {

            // Find the "filename" attribute within the Content-Disposition header parameter map
            auto params_it = headers_it->second.params.find("filename");

            if (params_it != headers_it->second.params.end()) {
                filename = params_it->second; // Captures original name like "historical_prices.csv"
            }
        }

        // 5. Save the file data (the raw binary payload is stored completely in file_part.body)
        std::ofstream out_file(filename, std::ios::binary);

        if (!out_file) {
            return crow::response(500, "Internal server error: Failed to write file to disk.");
        }
        
        out_file << file_part.body;
        out_file.close();

        // 6. Build response payload
        crow::json::wvalue response_json;
        response_json["status"] = "success";
        response_json["saved_as"] = filename;
        response_json["size_bytes"] = file_part.body.size();
        response_json["associated_asset"] = asset;

        return crow::response(200, response_json);
    });

    //------------
    // Middleware

    // Testing with:
    //   curl -k -i https://localhost:18080/api/dashboard
    //   curl -k -i -H "X-API-Key: secret_trading_token_123" https://localhost:18080/api/dashboard

    // Secure Dashboard Route (Automatically guarded by ApiGuardMiddleware)
    CROW_ROUTE(app, "/api/dashboard").CROW_MIDDLEWARES(app, ApiGuard)([]() {
        crow::json::wvalue response;
        response["portfolio_value"] = 125000.75;
        response["status"] = "active";
        return response;
    });

    //--------
    // Static

    // Testing with:
    //   curl -k -i https://localhost:18080/static/favicon.png --output favicon.png

    //---------
    // Session

    // Testing with:
    //   mkdir -p /tmp/sessiondata
    //   curl -k -i -X POST https://127.0.0.1:18080/api/login -c cookies.txt
    //   cat cookies.txt
    //   cat /tmp/sessiondata/*
    //   curl -k -i https://127.0.0.1:18080/api/resource -b cookies.txt

    // Route to establish/write to the session
    CROW_ROUTE(app, "/api/login").methods(crow::HTTPMethod::POST)([&app](const crow::request& req) {

        // Extract the built-in session context using the type alias
        auto& session = app.get_context<Session>(req);

        // Crow's native session maps act as a dynamically-typed dictionary 
        // that handles strings, integers, doubles, and booleans.
        session.set("username", "Alice");
        session.set("is_authenticated", true);
        session.set("failed_attempts", 0);

        return crow::response(200, "Session variables committed to FileStore!");
    });

    // Protected route to inspect and alter the session state
    CROW_ROUTE(app, "/api/resource")([&app](const crow::request& req) {

        auto& session = app.get_context<Session>(req);

        // Reading values using fallbacks: .get("key", fallback_value)
        std::string user = session.get("username", "Guest");
        bool is_auth = session.get("is_authenticated", false);

        crow::json::wvalue response;
        if (!is_auth) {
            response["status"] = "Access Denied";
            return crow::response(401, response);
        }

        // Use .apply() for safe, atomic read-and-modify alterations on the data.
        // This avoids race conditions across overlapping multithreaded requests.
        session.apply("dashboard_refresh_count", [](int current_val) {
            return current_val + 1;
        });

        response["status"] = "Authorized";
        response["user"] = user;
        response["total_visits"] = session.get("dashboard_refresh_count", 0);
        
        return crow::response(200, response);
    });

    //--------
    // Cookie

    // Testing with: 
    //   curl -k -i -X POST https://127.0.0.1:18080/api/set_cookie -c cookies.txt
    //   curl -k -i https://127.0.0.1:18080/api/get_cookie -b cookies.txt

    CROW_ROUTE(app, "/api/set_cookie").methods(crow::HTTPMethod::POST)([&app](const crow::request& req) {

        auto& cookie_ctx = app.get_context<crow::CookieParser>(req);

        // Build and drop the cookie
        cookie_ctx.set_cookie("session_token", "crypto_jwt_secure_string_abc123")
                  .path("/")        // Accessible across the entire API space
                  .max_age(300)     // Valid for 5 minutes (300 seconds)
                  .httponly()       // Protect against Cross-Site Scripting (XSS)
                  .secure();        // Transmit only over encrypted HTTPS connections

        return crow::response(200, "Success: Cookie 'session_token' has been dropped!\n");
    });

    CROW_ROUTE(app, "/api/get_cookie")([&app](const crow::request& req) {

        auto& cookie_ctx = app.get_context<crow::CookieParser>(req);

        // Read the incoming cookie value
        std::string token = cookie_ctx.get_cookie("session_token");

        crow::json::wvalue response_json;

        // Check if the cookie is missing or expired
        if (token.empty()) {
            response_json["status"] = "error";
            response_json["message"] = "Cookie 'session_token' not found or expired.";
            return crow::response(400, response_json);
        }

        // Return the cookie values found inside the browser request payload
        response_json["status"] = "success";
        response_json["retrieved_token"] = token;
        response_json["message"] = "Cookie read successfully from request headers.";
        
        return crow::response(200, response_json);
    });

    //------
    // CORS

    // Testing with:
    //   curl -k -i -X OPTIONS https://localhost:18080/api/data -H "Origin: http://random-website.com" -H "Access-Control-Request-Method: POST"
    //   curl -k -i -X OPTIONS https://localhost:18080/cors/secure-assets -H "Origin: example.com" -H "Access-Control-Request-Method: POST"

    /*
     * Expected outcome
     *
     * Access-Control-Allow-Origin returns * (wildcard) because no specific origin rule overrides the
     * generic /api/data route:
     * 
     * > curl -k -i -X OPTIONS https://localhost:18080/api/data -H "Origin: http://random-website.com" -H "Access-Control-Request-Method: POST"
     *     HTTP/1.1 204 No Content
     *     Access-Control-Allow-Origin: *
     *     Access-Control-Allow-Headers: X-Custom-Header, Upgrade-Insecure-Requests, Content-Type
     *     Access-Control-Allow-Methods: POST, GET
     *     Allow: OPTIONS, HEAD, GET
     *     Content-Length: 0
     *     Server: Crow/master
     *
     * The middleware successfully locked the scope to example.com:
     * 
     * > curl -k -i -X OPTIONS https://localhost:18080/cors/secure-assets -H "Origin: example.com" -H "Access-Control-Request-Method: POST"
     *     HTTP/1.1 204 No Content
     *     Access-Control-Allow-Origin: example.com
     *     Access-Control-Allow-Headers: *
     *     Access-Control-Allow-Methods: *
     *     Allow: OPTIONS, HEAD, GET
     *     Content-Length: 0
     *     Server: Crow/master
     */

    CROW_ROUTE(app, "/api/data")([](){
        crow::json::wvalue res;
        res["status"] = "success";
        res["scope"] = "global api context";
        return crow::response(200, res);
    });

    // Restricted Route (Falls under the "/cors" prefix rule: only example.com can access)
    CROW_ROUTE(app, "/cors/secure-assets")([](){
        crow::json::wvalue res;
        res["status"] = "success";
        res["scope"] = "restricted to example.com domain only";
        return crow::response(200, res);
    });
}
