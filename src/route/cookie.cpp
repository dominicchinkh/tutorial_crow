#include <string>

#include "route.h"

using namespace std;

void register_cookie_routes(CrowApp& app) 
{
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
}
