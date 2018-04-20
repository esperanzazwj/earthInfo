//! macro headers do not use #pragma once
#include "compat/compiler.macro.hpp"

#ifdef SS_UTIL_NON_THROWING_UNREACHABLE
    #if defined(SS_UTIL_GCC_COMPATIBLE)
        #define SS_UTIL_UNREACHABLE() __builtin_unreachable()
    #elif defined(SS_UTIL_MSVC_COMPATIBLE)
        #define SS_UTIL_UNREACHABLE() __assume(false)
    #else
        #error "Unsupported compiler"
    #endif
#else
    #include <stdexcept>
    #define SS_UTIL_UNREACHABLE() throw ::std::logic_error{"unreachable code reached"}
#endif

#include "compat/compiler.undef.hpp"

