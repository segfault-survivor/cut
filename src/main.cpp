#include <cut/args.hpp>
#include <cut/cl.hpp>
#include <cut/clang.hpp>
#include <cut/hash.hpp>
#include <cut/io.hpp>
#include <cut/log.hpp>
#include <cut/pch.hpp>
#include <cut/time.hpp>

#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <stdexcept>
#include <string>

inline auto generate_preamble //
    (                         //
        args                          a,
        std::filesystem::path const & dir,
        std::filesystem::path const & exe,
        std::filesystem::path const & cpp //
    )
{
    a.push_back("-Xclang");
    a.push_back("-print-preamble");

    auto const preamble     = dir / std::format("cut-{}-{}.preamble.cpp", cpp.stem().string(), hash(to_string(a)));
    auto       preamble_new = std::filesystem::path{preamble.string() + ".new"};

    // *std::next(std::ranges::find(a, "-o")) = pre; // -print-preamble ignores -o
    execute(make_command(exe, a, " > " + preamble_new.string()));

    if (!std::filesystem::exists(preamble) || is_different_content(preamble, preamble_new))
    {
        std::filesystem::remove(preamble);
        std::filesystem::rename(preamble_new, preamble);
    }
    else
    {
        std::filesystem::remove(preamble_new);
    }

    return preamble;
}

inline auto generate_pch //
    (                    //
        auto                          compiler,
        args                          a,
        std::filesystem::path const & exe,
        std::filesystem::path const & cpp,
        std::filesystem::path const & preamble,
        bool                          force = false //
    )
{
    auto const pch = std::filesystem::path{preamble.string() + ".pch"};

    if                                                                                            //
        (                                                                                         //
            force ||                                                                              //
            !std::filesystem::exists(pch) ||                                                      //
            std::filesystem::last_write_time(pch) < std::filesystem::last_write_time(preamble) || //
            tlog(compiler, pch)                                                                   //
        )
    {
        do_generate_pch(compiler, std::move(a), exe, cpp, preamble, pch);
    }
    else
    {
        // skip
    }

    return pch;
}

inline auto get_cpp(auto compiler, args const & a) noexcept
{
    auto const is_cpp = [&](auto const & f)
    {
        return !f.starts_with("-") && f.ends_with(".cpp");
    };
    return is_compilation(compiler, a) ? std::views::filter(a, is_cpp) | std::ranges::to<args>() : args{};
}

inline auto get_output_dir(auto compiler, args const & a)
{
    return absolute(std::filesystem::path{get_obj(compiler, a)}).parent_path();
}

inline auto run(auto compiler, args const & a)
{
    auto const exe = get_exe(compiler);

    if (auto const cpp = get_cpp(compiler, a); size(cpp) == 1)
    {
        auto const source   = cpp[0];
        auto const preamble = generate_preamble(a, get_output_dir(compiler, a), get_exe_pre(compiler), source);
        auto const pch      = generate_pch(compiler, a, exe, source, preamble);

        switch (use_pch(compiler, a, exe, preamble, pch, source))
        {
            using enum use_pch_result;
            case ok:
            {
            }
            break;
            case need_rebuild:
            {
                auto const pch = generate_pch(compiler, a, exe, source, preamble, true);
                use_pch(compiler, a, exe, preamble, pch, source);
            }
            break;
        }
    }
    else
    {
        pass_through(exe, a);
    }
}

inline auto print_error(auto const & e)
{
    std::println(std::cerr, "cut! error: {}", e.what());
}

inline auto on_exception(auto const & e)
{
    print_error(e);
    return EXIT_FAILURE;
}

auto main(int argc, char * argv[]) -> int
try
{
    auto const p = print_duration{"cut!"};

    auto const exe = std::filesystem::path{argv[0]};
    auto const a   = args{argv + 1, argv + argc};

    is_verbose = has(a, "-v");

    auto is = [](auto p, auto s)
    {
        return p.stem().string().contains(s);
    };

    if (is(exe, "cut-clang++"))
    {
        run(clang{}, a);
    }
    else if (is(exe, "cut-cl"))
    {
        run(cl{}, a);
    }
    else
    {
        throw std::runtime_error("What am I?");
    }

    return EXIT_SUCCESS;
}
catch (execution_error const & e)
{
    print_error(e);

    return e.code;
}
catch (std::exception const & e)
{
    return on_exception(e);
}
catch (...)
{
    return on_exception(std::runtime_error("Unknown"));
}
