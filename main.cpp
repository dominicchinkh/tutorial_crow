#define CROW_JSON_USE_MAP

#include <iomanip>
#include <iostream>
#include <vector>

#include "crow.h" 
#include "crow/middlewares/cookie_parser.h" // Required dependency
#include "crow/middlewares/cors.h"
#include "crow/middlewares/session.h"       // Built-in session header

#include "src/logger/custom_logger.h"
#include "src/middleware/api_guard.h"
#include "src/route/route.h"

using namespace std;

// Create a clean type alias exactly matching your architectural snippet
using Session = crow::SessionMiddleware<crow::FileStore>;

int main(int argc, char *argv[]) 
{
    //--------------------
    // App initialization

    // The App (or SimpleApp) class organizes all the different parts of Crow and provides the developer 
    // a simple interface to interact with these parts. 
    // crow::SimpleApp app;

    // Pass your middleware class as a template parameter inside App initialization.
    // The order of definition dictates the execution execution chain sequence.
    crow::App<crow::CookieParser, Session, ApiGuard, crow::CORSHandler> app{
        crow::CookieParser{},
        Session{crow::FileStore{"/tmp/sessiondata"}}
    };

    //-----
    // SSL
    app.ssl_file("certs/server.crt", "certs/server.key");

    //------
    // CORS

    auto& cors = app.get_middleware<crow::CORSHandler>();
    
    cors.global()
        // Allow specific custom headers from cross-origin requests
        .headers("X-Custom-Header", "Upgrade-Insecure-Requests", "Content-Type")
        // Allow only POST and GET requests globally
        .methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET);
    
    cors.prefix("/cors")
        // Overrides permissions specifically for routes starting with /cors
        .origin("example.com")
    ;

    //---------
    // Logging

    // https://crowcpp.org/master/guides/logging/
    app.loglevel(crow::LogLevel::Info);

    CustomLogger logger;
    crow::logger::setHandler(&logger);

    //------------
    // Add routes
    register_api_routes(app);

    //-----------------
    // Catch all route

    CROW_CATCHALL_ROUTE(app)([](){
        return crow::response(crow::status::NOT_FOUND, "Sorry, page not found :-)");;
    });

    // Run the app
    app
      //.bindaddr("192.168.1.2")
      .port(18080)
      //.ssl_file("certfile.crt","keyfile.key")
      .multithreaded()
      .run();

    return 0;
}
