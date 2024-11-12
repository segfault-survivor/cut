#pragma once

#include <cut/args.hpp>
#include <cut/log.hpp>
#include <cut/string.hpp>

#include <filesystem>
#include <stdexcept>
#include <string>

struct execution_error : std::runtime_error
{
    int const code;

    execution_error(std::string const & command, int code) : std::runtime_error{command}, code{code}
    {
    }
};

inline auto system(std::string const & s)
{
    return std::system //
        (
#if defined(_WIN32)
            quote
#endif
            (s)
                .c_str() //
        );
}

inline auto execute(std::string const & s)
{
    verbose("cut! command: {}", s);
    auto const result = system(s);
    verbose("cut! result: {}", result);
    if (EXIT_SUCCESS == result)
    {
        // good
    }
    else
    {
        throw execution_error{s, result};
    }
}

inline auto make_command(std::filesystem::path const & exe, args const & a, std::string const & tail = "")
{
    return quote_if_spaces(exe.string()) + " " + to_string(a) + " " + tail;
}

inline auto pass_through(std::filesystem::path const & exe, args const & a)
{
    execute(make_command(exe, a));
}
