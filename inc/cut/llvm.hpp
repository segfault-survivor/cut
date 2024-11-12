#pragma once

#include <filesystem>

inline auto llvm_bin_dir(auto exe)
{
    return
#if defined(LLVM_DIR)
        std::filesystem::path{LLVM_DIR} / "bin" /
#endif
        exe;
}
