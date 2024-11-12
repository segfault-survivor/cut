#pragma once

#include <stdexcept>
#include <string>

inline auto quote(std::string const & s)
{
    return "\"" + s + "\"";
}
