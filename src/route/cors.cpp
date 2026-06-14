#include <string>

#include "route.h"

using namespace std;

void register_cors_routes(CrowApp& app) 
{
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
