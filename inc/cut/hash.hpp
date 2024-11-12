#pragma once

#include <type_traits>
#include <vector>

inline auto hash(auto const & a) noexcept
{
    return std::hash<std::remove_cvref_t<decltype(a)>>{}(a);
}
