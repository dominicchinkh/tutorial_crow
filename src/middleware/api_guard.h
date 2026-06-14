#ifndef API_GUARD_H
#define API_GUARD_H

#include <chrono>
#include <iostream>
#include <string>
#include "crow.h"

struct ApiGuard : crow::ILocalMiddleware {

    // Structural required types for Crow's middleware engine
    struct context {};

    // Executes BEFORE the route handler handles the request
    void before_handle(crow::request& req, crow::response& res, context& ctx) {

        // Example A: A simple request logger
        std::cout << "[LOG] Incoming " << crow::method_name(req.method)
                  << " request to: " << req.url << std::endl;

        // Example B: API Key Authentication Guard
        // We look for a header named "X-API-Key"
        std::string api_key = req.get_header_value("X-API-Key");

        if (api_key != "secret_trading_token_123") {
            // Short-circuit the request: Set a 401 status and complete the response early
            res.code = 401;
            res.write("Unauthorized: Invalid or missing X-API-Key header.");
            
            // Calling res.end() halts the chain and prevents executing subsequent 
            // middleware layers or the main route handler.
            res.end(); 
        }
    }

    // Executes AFTER the route handler completes its execution
    void after_handle(crow::request& req, crow::response& res, context& ctx) {

        // Inject global security or CORS headers to all outgoing responses
        res.add_header("X-Server-Framework", "C++ Crow Server");
        res.add_header("Access-Control-Allow-Origin", "*");
        
        std::cout << "[LOG] Response status sent: " << res.code << std::endl;
    }
};

#endif
