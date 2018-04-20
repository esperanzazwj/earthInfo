//! macro headers do not use #pragma once
// OS compatibility macros

#if defined(__linux__) || defined(__MINGW32__)      // Linux, MinGW, MSYS2
    #define SS_UTIL_LINUX_COMPATIBLE
    #if defined(ANDROID)
        #define SS_UTIL_ANDROID_COMPATIBLE
    #endif
#elif defined(_WIN32)                               // Windows
    #define SS_UTIL_WINDOWS_COMPATIBLE
#elif defined(__EMSCRIPTEN__)                       // asm.js, WebAssembly
    #define SS_UTIL_WEB_COMPATIBLE
#else
    #error "Only support Linux, Windows, and Web platforms."
#endif

