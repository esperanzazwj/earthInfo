// #pragma once     // no pragma once intentionally
#ifdef SS_LIB_EXPOSE_NATIVE
    #include "../polyfill/android.hpp"
#endif

#include "../util/compat/gl.macro.hpp"
#include "../util/compat/os.macro.hpp"

#ifdef SS_UTIL_GL_REQUIRES_ES
    #define GLFW_INCLUDE_ES31
    #ifdef SS_LIB_EXPOSE_NATIVE
        #define GLFW_EXPOSE_NATIVE_EGL
    #endif
    #ifndef SS_UTIL_WEB_COMPATIBLE
        #ifdef SS_LIB_EXPOSE_NATIVE
            #define GLFW_EXPOSE_NATIVE_EGL
            #include <EGL/egl.h>
            #include <EGL/eglext.h>
        #endif
        #include <GLES3/gl31.h>
    #endif
#else
    #define GLEW_NO_GLU
    #include <GL/glew.h>
    #undef GLEW_NO_GLU
#endif

#ifdef SS_LIB_EXPOSE_NATIVE
    #ifdef SS_UTIL_WINDOWS_COMPATIBLE
        #define GLFW_EXPOSE_NATIVE_WIN32
    #endif
    #ifdef SS_UTIL_WEB_COMPATIBLE
        #include <emscripten.h>
    #endif
#endif

#include <GLFW/glfw3.h>

#ifndef SS_UTIL_WEB_COMPATIBLE
    #include <GLFW/glfw3native.h>
#endif

#ifdef GLFW_EXPOSE_NATIVE_EGL
    #undef GLFW_EXPOSE_NATIVE_EGL
#endif

#ifdef GLFW_EXPOSE_NATIVE_WIN32
    #undef GLFW_EXPOSE_NATIVE_WIN32

    // Prevent dangerous macros leaking out of <windows.h>
    #undef min
    #undef max
    #undef near
    #undef far
    #undef pascal
    #undef cdecl
    #undef small
    #undef byte
    #undef interface
#endif

#ifdef GLFW_INCLUDE_ES31
    #undef GLFW_INCLUDE_ES31
#endif

#include "../util/compat/gl.undef.hpp"
#include "../util/compat/os.undef.hpp"

