#include "logging.h"

#include <iostream>

namespace U
{
    template <>
    void Log<char>(const char* Value)
    {
        std::cout << std::string_view(Value) << std::endl;
    }
}