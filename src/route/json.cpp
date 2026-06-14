#include <string>

#include "route.h"

using namespace std;

void register_json_routes(CrowApp& app) 
{
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
}
