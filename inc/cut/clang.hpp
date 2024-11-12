#pragma once

#include <cut/args.hpp>
#include <cut/exec.hpp>
#include <cut/io.hpp>
#include <cut/llvm.hpp>
#include <cut/path.hpp>
#include <cut/pch.hpp>

#include <algorithm>
#include <filesystem>

struct clang
{
};

inline auto is_compilation(clang, args const & a) noexcept
{
    auto const o = std::ranges::find(a, "-o");
    return has(a, "-c") && o != a.end() && std::next(o) != a.end();
}

inline auto do_generate_pch //
    (                       //
        clang,
        args                          a,
        std::filesystem::path const & exe,
        std::filesystem::path const & cpp,
        std::filesystem::path const & preamble,
        std::filesystem::path const & pch //
    )
{
    *std::next(std::ranges::find(a, "-o")) = pch.string();
    *std::ranges::find(a, "-c")            = "-xc++-header";
    *std::ranges::find(a, cpp.string())    = preamble.string();

    execute(make_command(exe, a, "-fpch-instantiate-templates -I" + absolute(cpp).parent_path().string()));
}

inline auto use_pch //
    (               //
        clang,
        args                          a,
        std::filesystem::path const & exe,
        std::filesystem::path const & preamble,
        std::filesystem::path const & pch,
        std::filesystem::path const & cpp //
    )
{
    a.push_back("-include-pch");
    a.push_back(pch.string());
    a.push_back("-Xclang");
    a.push_back(std::format("-preamble-bytes={},1", std::filesystem::file_size(preamble)));

    auto const l = append(pch, ".log");

    try // TODO: use -verify-pch?
    {
        execute(make_command(exe, a, std::format("-serialize-diagnostics {}", l.string())));
        return use_pch_result::ok;
    }
    catch (std::exception const &)
    {
        return read_all(l).contains("please rebuild precompiled header") ? use_pch_result::need_rebuild : throw;
    }
}

inline auto tlog(clang, std::filesystem::path const &)
{
    return false;
}

inline auto get_obj(clang, args const & a)
{
    return *std::next(std::ranges::find(a, "-o"));
}

inline auto get_exe(clang)
{
    return llvm_bin_dir //
        (               //
            "clang++"
#if defined(_WIN32)
            ".exe" //
#endif
        );
}

inline auto get_exe_pre(clang c)
{
    return get_exe(c);
}
