#pragma once

#include <cut/args.hpp>
#include <cut/exec.hpp>
#include <cut/io.hpp>
#include <cut/llvm.hpp>
#include <cut/path.hpp>
#include <cut/pch.hpp>

#include <algorithm>
#include <filesystem>
#include <print>

struct cl
{
};

inline auto is_Fo(args::value_type const & v) noexcept
{
    return v.starts_with("/Fo");
}

template <typename T>
inline auto find_Fo(T && a) noexcept
{
    return std::ranges::find_if(std::forward<T>(a), is_Fo);
}

inline auto preamble_to_cpp_obj_and_lib(std::filesystem::path const & preamble)
{
    auto const cpp = append(preamble, ".i.cpp");
    auto const obj = replace_extension(cpp, ".obj");
    auto const lib = replace_extension(cpp, ".lib");

    return std::tuple{cpp, obj, lib};
}

inline auto get_tlog_dir(std::filesystem::path const & pch)
{
    return std::filesystem::path{pch.string() + ".tlog"};
}

inline auto get_tlog_read(std::filesystem::path const & pch)
{
    return get_tlog_dir(pch) / "cl.read.1.tlog";
}

inline auto tracker(std::string const & cmd, std::filesystem::path const & dir, std::filesystem::path const & cpp)
{
    return std::format("tracker /i {} /r {} /c {}", dir.string(), cpp.string(), cmd);
}

inline auto get_pch_incomplete(std::filesystem::path const & pch)
{
    return append(pch, ".incomplete");
}

inline auto do_generate_pch //
    (                       //
        cl,
        args                          a,
        std::filesystem::path const & exe,
        std::filesystem::path const & cpp,
        std::filesystem::path const & preamble,
        std::filesystem::path const & pch //
    )
{
    binary_output(get_pch_incomplete(pch));

    auto [pch_cpp, pch_obj, pch_lib] = preamble_to_cpp_obj_and_lib(preamble);

    *find_Fo(a) = "/Fo" + pch_obj.string();

    {
        auto o = binary_output(pch_cpp);
        std::println(o, R"raw(#include "{}")raw", preamble.string());
    }

    *std::ranges::find(a, cpp.string()) = pch_cpp.string();
    a.push_back(std::format(R"raw(/Yc"{}")raw", preamble.string()));
    a.push_back(std::format(R"raw(/Fp"{}")raw", pch.string()));

    std::filesystem::remove(get_tlog_read(pch));
    execute(tracker(make_command(exe, a, "-I" + absolute(cpp).parent_path().string()), get_tlog_dir(pch), pch_cpp));

    auto options = has(a, "/nologo") ? "/nologo" : "";
    execute(std::format("lib {} /OUT:{} {}", options, pch_lib.string(), pch_obj.string()));

    std::filesystem::remove(get_pch_incomplete(pch));
}

inline auto utf16le_to_utf8(std::filesystem::path const & f, std::filesystem::path const & t)
{
    execute(make_command("type", {f.string()}, " > " + t.string())); // codecvt at home
}

inline auto tlog(cl, std::filesystem::path const & pch)
{
    if (std::filesystem::exists(get_pch_incomplete(pch)))
    {
        return true;
    }

    auto const f = get_tlog_read(pch);
    auto const t = append(f, ".txt");
    utf16le_to_utf8(f, t);

    auto const needs_update = [&](std::filesystem::path const & l)
    {
        auto const s = l.string();

        return !s.starts_with("#Command") && //
               !s.starts_with("^") &&        //
               !s.ends_with(".DLL") &&       //
               exists(l) &&                  //
               last_write_time(pch) < last_write_time(l);
    };

    return std::ranges::any_of(read_lines(t), needs_update);
}

inline auto use_pch //
    (               //
        cl                            compiler,
        args                          a,
        std::filesystem::path const & exe,
        std::filesystem::path const & preamble,
        std::filesystem::path const & pch,
        std::filesystem::path const & cpp //
    )
{
    a.push_back(std::format("/Yu{}", preamble.string()));
    a.push_back(std::format("/Fp{}", pch.string()));

    auto const rest = append(preamble, ".rest.cpp");
    {
        auto i = binary_input(cpp);
        i.seekg(std::filesystem::file_size(preamble));

        auto o = binary_output(rest);
        std::println(o, R"raw(#include "{}")raw", preamble.generic_string());

        auto [pch_cpp, pch_obj, pch_lib] = preamble_to_cpp_obj_and_lib(preamble);
        std::println(o, R"raw(#pragma comment(lib, "{}"))raw", pch_lib.generic_string());
        std::println(
            o, R"raw(#line {} "{}")raw", count_lines(preamble), absolute(std::filesystem::path{cpp}).generic_string());

        o << i.rdbuf();
    }
    *std::ranges::find(a, cpp.string()) = rest.string();

    execute(make_command(exe, a));
    return use_pch_result::ok; // TODO: always ok?
}

inline auto get_obj(cl, args const & a)
{
    return find_Fo(a)->substr(std::size("/Fo") - 1);
}

inline auto is_compilation(cl, args const & a) noexcept
{
    return has(a, "/c") && find_Fo(a) != a.end();
}

inline auto get_exe(cl)
{
    return std::filesystem::path{"cl.exe"};
}

inline auto get_exe_pre(cl)
{
    return llvm_bin_dir("clang-cl.exe");
}
