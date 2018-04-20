//! macro headers do not use #pragma once
// Compiler compatibility macros

#if defined(__GNUC__)       // All of gcc, clang and icc define this
    #define SS_UTIL_GCC_COMPATIBLE
#elif defined(_MSC_VER)     // MSVC defines this
    #define SS_UTIL_MSVC_COMPATIBLE
#else
    #error "Only support MSVC, gcc, clang and icc."
#endif

