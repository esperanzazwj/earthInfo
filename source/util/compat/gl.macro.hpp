//! macro headers do not use #pragma once
// GL compatibility macros

#include "os.macro.hpp"
#if defined(SS_UTIL_ANDROID_COMPATIBLE) || defined(SS_UTIL_WEB_COMPATIBLE)
    #define SS_UTIL_GL_REQUIRES_ES
#endif
#include "os.undef.hpp"

