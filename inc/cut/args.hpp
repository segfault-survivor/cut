#pragma once

#include <cut/string.hpp>

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

using args = std::vector<std::string>;

inline auto has(args const & a, std::string const & v) noexcept
{
    return std::ranges::contains(a, v);
}

inline auto quote_if_spaces(std::string const & s)
{
    auto const has_spaces = s.contains(' ');
    auto const has_quotes = s.contains("\"");
    if (has_quotes && has_spaces)
    {
        throw std::runtime_error("quotes mixed with spaces are not supported");
    }
    return has_spaces ? quote(s) : s;
}

inline auto to_string(args const & a)
{
    if (a.empty())
    {
        return std::string{};
    }
    else
    {
        auto result = a[0];
        for (auto && i : a | std::views::drop(1))
        {
            result += " " + quote_if_spaces(i);
        }
        return result;
    }

    // return a | std::views::join_with(' ') | std::ranges::to<std::string>();
}
