#include <string>

#include "route.h"
#include "../entity/employee.h"

using namespace std;

void register_basic_routes(CrowApp& app) 
{
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

    //--------
    // Static

    // Testing with:
    //   curl -k -i https://localhost:18080/static/favicon.png --output favicon.png
}
