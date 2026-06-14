#include <string>

#include "route.h"

using namespace std;

void register_middleware_routes(CrowApp& app) 
{
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
}
