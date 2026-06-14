#include <string>

#include "route.h"

using namespace std;

void register_session_routes(CrowApp& app) 
{
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
}
