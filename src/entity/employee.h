
#pragma once

#include <string>
#include "crow.h" 

class Employee: public crow::returnable
{
    public:
        Employee(const std::string & n): returnable("text/plain"), name {n}
        {
        }

        std::string dump() const override
        {
            return name;
        }

    private:
        const std::string name;

};
