//! macro headers do not use #pragma once
// OS compatibility macros

#ifdef SS_UTIL_LINUX_COMPATIBLE
    #undef SS_UTIL_LINUX_COMPATIBLE
#endif

#ifdef SS_UTIL_ANDROID_COMPATIBLE
    #undef SS_UTIL_ANDROID_COMPATIBLE
#endif

#ifdef SS_UTIL_WINDOWS_COMPATIBLE
    #undef SS_UTIL_WINDOWS_COMPATIBLE
#endif

