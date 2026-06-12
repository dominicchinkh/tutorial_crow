#include <iomanip>
#include <iostream>
#include <vector>
#include "crow.h" 

using namespace std;

int main(int argc, char *argv[]) 
{
    // The App (or SimpleApp) class organizes all the different parts of Crow and provides the developer 
    // a simple interface to interact with these parts. 
    crow::SimpleApp app;

    // Add a route
    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });

    // Run the app
    app.port(18080).multithreaded().run();

    return 0;
}
