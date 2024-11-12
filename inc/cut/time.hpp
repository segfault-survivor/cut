#pragma once

#include <chrono>
#include <iostream>
#include <print>

inline auto now()
{
    return std::chrono::high_resolution_clock::now();
}

template <typename P, typename... A>
inline auto to(std::chrono::duration<A...> d)
{
    return std::chrono::duration_cast<P>(d).count();
}

class print_duration
{
    decltype(now()) const time = now(); // /ˈƆː.TƏƱ/
    std::string const     what;

public:
    explicit print_duration(std::string what) : what{std::move(what)}
    {
    }

    ~print_duration()
    {
        auto const d = now() - time;
        using namespace std::chrono;
        std::println //
            (        //
                std::cout,
                "{} took {} seconds", //
                what,
                to<milliseconds>(d) / 1000.0 //
            );
    }
};
