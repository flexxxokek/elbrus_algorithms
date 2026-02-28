#pragma once

#include <mystruct.hpp>

#include <stdint.h>
// #include <stdtype

typedef uint64_t u64;
typedef int64_t i64;


struct UnixPath
{
    constexpr static const u64 MY_PATH_MAX = 4096;
    constexpr static const char* ERROR_STRING = "Going upper than the root directory is not possible.\n";

    String solve(const char* inputPath) const;
};
