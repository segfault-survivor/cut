#pragma once

#include <filesystem>

inline auto append(std::filesystem::path p, auto tail)
{
    p += tail;
    return p;
}

inline auto replace_extension(std::filesystem::path p, auto e) // Am I not living right?
{
    p.replace_extension(e);
    return p;
}
