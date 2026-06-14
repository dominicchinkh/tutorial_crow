
#pragma once

#include "crow.h"

class CustomLogger : public crow::ILogHandler {
    public:
        CustomLogger() {}

        void log(const std::string& message, crow::LogLevel level) {

            // "message" doesn't contain the timestamp and loglevel prefix the default 
            // logger does and it doesn't end in a newline.
            std::cerr << "My custom logger: " << message << std::endl;
        }
};
