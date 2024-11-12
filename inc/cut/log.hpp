#pragma once

#include <iostream>
#include <print>

inline auto is_verbose = bool{};

template <class... T>
inline auto verbose(std::format_string<T...> const format, T &&... args)
{
    if (is_verbose)
    {
        std::println(std::cout, format, std::forward<T>(args)...);
    }
}
