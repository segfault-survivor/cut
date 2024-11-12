#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>

inline auto binary_input(std::filesystem::path const & p)
{
    auto result = std::ifstream{p, std::ios_base::binary};
    result.exceptions(std::ios_base::failbit | std::ifstream::badbit | std::ifstream::eofbit);
    return result;
}

inline auto binary_output(std::filesystem::path const & p)
{
    auto result = std::ofstream{p, std::ios_base::binary};
    result.exceptions(std::ios_base::failbit | std::ifstream::badbit);
    return result;
}

inline auto read_all(std::filesystem::path const & p)
{
    auto s = std::ostringstream{};
    s << binary_input(p).rdbuf();
    return s.str();
}

inline auto is_different_content(std::filesystem::path const & a, std::filesystem::path const & b)
{
    return read_all(a) != read_all(b);
}

inline auto read_lines(std::filesystem::path const & p)
{
    auto result = std::vector<std::string>{};
    auto i      = std::ifstream{p};
    for (auto l = std::string{}; std::getline(i, l);)
    {
        while (l.ends_with('\n') || l.ends_with('\r'))
        {
            l.pop_back();
        }
        result.push_back(l);
    }
    return result;
}

inline auto count_lines(std::filesystem::path const & p)
{
    auto i      = std::ifstream{p};
    auto result = std::size_t{1};
    for (auto l = std::string{}; std::getline(i, l); ++result)
    {
    }
    return result;
}
