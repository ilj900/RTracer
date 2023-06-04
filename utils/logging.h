#pragma once

#include <iostream>
#include <string>

namespace U
{
    template <typename T>
    void Log(const T& Value)
    {
        std::cout << Value << std::endl;
    }

    template <typename T>
    void Log(const T* Value)
    {
        std::cout << *Value << std::endl;
    }

    template <>
    void Log<char>(const char* Value);
}