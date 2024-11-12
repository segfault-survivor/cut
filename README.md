![Cut! logo](logo.png)

![Cut! demo](demo.gif)

# Overview

Cut! speeds up C++ compilation by automatically generating a precompiled header (PCH), no code changes are required.
 
Cut! divides the source file into two parts:
- the first containing all `#include` directives; PCH is made from this part.
- the second part references this PCH, which makes compilation go faster.

Cut! is neither a patch nor a fork of clang. It uses the available clang command line arguments, however poorly documented.

# Requirements

- Clang 19
- Optional: MSVC, Catch2 and Boost

# Installation

## Clone the repo

    git clone https://github.com/segfault-survivor/cut.git
    cd cut

## Compile

### On Linux

    build.sh

will produce `cut-clang++`

### On Windows

    build.bat

will produce `cut-clang++.exe` and `cut-cl.exe`[^1].

# Usage

`cut-clang++` is a wrapper for `clang++`. 

When both `-c` and `-o` are provided:

    cut-clang++ -c source.cpp -o source.o

`cut-clang++` executes:

```
clang++ -c source.cpp -o source.o -Xclang -print-preamble  > cut-source-N.preamble.cpp.new
<update cut-source-N.preamble.cpp if needed>
clang++ -xc++-header cut-source-N.preamble.cpp -o cut-source-N.preamble.cpp.pch -fpch-instantiate-templates -I<source.cpp folder>
clang++ -c source.cpp -o source.o -include-pch cut-source-N.preamble.cpp.pch -Xclang -preamble-bytes=M,1 -serialize-diagnostics cut-source-N.preamble.cpp.pch.log
```

Any other `cut-clang++` invocation is passed to `clang++` unmodified:

    cut-clang++ source.cpp # this simply invokes clang++ source.cpp

# Benchmark

Cut! itself does not do much and the compilation speed depend on the speed of your compiler (with or without PCH). But nevertheless, benchmarks are necessary.

There is `test/run.bat`:

    run.bat [<output_dir>] [<Boost_dir>] [<Catch2_dir>]

Running it without parameters printed the following table on my potato PC:

| File|clang++, ms|cut-clang++ run#1, ms|cut-clang++ run#2, ms|
| --- | :---: | :---: | :---: |
| import_std|6027 |7438 |545 |
| import_windows|55992 |65589 |946 |
| cut!|4856 |6231 |1716 |

# Bigger benchmark

When launched with following parameters in MSVC Command Prompt:

    ...\test> run.bat z:/temp C:/lib/Boost_1_86 C:/lib/Catch2

| File|clang++, ms|cl, ms|cut-clang++ run#1, ms|cut-clang++ run#2, ms|cut-cl run#1, ms|cut-cl run#2, ms|
| --- | :---: | :---: | :---: | :---: | :---: | :---: |
| import_std|6022 |4524 |7370 |587 |5239 |389 |
| import_windows|56256 |80374 |63732 |860 |80305 |920 |
| import_catch|11319 |7285 |13787 |4478 |8586 |770 |
| catch_evt|2887 |2284 |3686 |743 |2872 |543 |
| import_boost|47362 |58964 |60898 |3498😡 |63097 |883😃 |
| stacktrace_test|3564 |2735 |4398 |814 |3314 |585 |
| random_snips|8702 |6572 |10287 |3400 |7479 |2581 |
| filtering_example|3166 |2618 |4110 |850 |3146 |601 |
| cut!|4844 |3980 |6100 |1602 |4572 |751 |

Here `clang++` and `cl` columns are normal compiler invocations for the reference.

`cut-clang++ run#1` and `cut-cl run#1` columns are first invocations when PCHs are generated, this is why it takes longer.

`cut-clang++ run#2` and `cut-cl run#2` columns is where PCH gets reused.

![Cut! benchmark](test/benchmark.png)

# FAQ
## That big table is already scary in itself. Why the angry face?

`clang` is 3498 ms / 883 ms ≈ 4 times slower than `cl` for the same end result.

## Should cut! be used for non-incremental builds?

No. It is not going to cut it.

[^1]: The only reason `cut-cl` exists is to demonstrate how slow `clang` is.
