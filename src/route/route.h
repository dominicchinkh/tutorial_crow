
#pragma once

#include "crow.h"
#include "crow/middlewares/cookie_parser.h"
#include "crow/middlewares/session.h"
#include "crow/middlewares/cors.h"

#include "../middleware/api_guard.h"

// Create a clean type alias matching your exact main app middleware configuration
using Session = crow::SessionMiddleware<crow::FileStore>;
using CrowApp = crow::App<crow::CookieParser, Session, ApiGuard, crow::CORSHandler>;

// Function declaration to register our standard routes
void register_api_routes(CrowApp& app);
