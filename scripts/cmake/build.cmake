project(ss-engine)

find_package(GLM REQUIRED)
find_package(Cereal REQUIRED)
find_package(Assimp REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(NativeGlue REQUIRED)
find_package(NDKHelper REQUIRED)

file(GLOB_RECURSE ss-engine-sources ${CMAKE_SOURCE_DIR}/source/*.cpp)

if (ANDROID)
    add_library(ss-engine SHARED ${ss-engine-sources})
    target_compile_options(
        ss-engine PRIVATE
        -std=c++14
    )
else ()
    add_executable(ss-engine ${ss-engine-sources})
    target_compile_features(ss-engine PUBLIC cxx_std_14)
    set_property(
        TARGET ss-engine
        PROPERTY CXX_EXTENSIONS OFF
    )
    target_include_directories(ss-engine SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/source/override/assert)
endif ()

set_property(
    TARGET ss-engine
    PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

target_link_libraries(
    ss-engine
    GLM
    Cereal
    Assimp
    GLFW3
    NativeGlue
    NDK_Helper_Gesture_Detector
)

if (EMSCRIPTEN)
    # Workaround for setting options for Emscripten
    set_property(
        TARGET ss-engine
        APPEND_STRING PROPERTY LINK_FLAGS " -O3 -s BINARYEN=1 -s DISABLE_EXCEPTION_CATCHING=0 -s ALLOW_MEMORY_GROWTH=1 --no-heap-copy"
    )
    set_property(
        TARGET ss-engine
        APPEND_STRING PROPERTY LINK_FLAGS " -s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1"
    )
    set_property(
        TARGET ss-engine
        APPEND_STRING PROPERTY LINK_FLAGS " --preload-file ${CMAKE_SOURCE_DIR}/bin/runtime@runtime --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html"
    )
    set_property(
        TARGET ss-engine
        PROPERTY SUFFIX .html
    )
elseif (ANDROID)
    target_link_libraries(
        ss-engine
        android
        log
        EGL
        GLESv3
    )
    set_property(
        TARGET ss-engine
        APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate"
    )
else ()
    # These are only required on Desktop platforms
    if (MSVC)
        find_package(GLEW-Windows REQUIRED)
    else ()
        find_package(GLEW REQUIRED)
    endif ()
    find_package(OpenGL REQUIRED)
    target_link_libraries(
        ss-engine
        GLEW::GLEW
        OpenGL::GL
    )
endif ()

if (MSVC)
    target_compile_options(
        ss-engine PRIVATE
        -utf-8
        -arch:AVX2                  # Support for SIMD
        -D_SCL_SECURE_NO_WARNINGS   # Remove Microsoft-only deprecation warnings
        -wd4297                     # No warnings when throwing in dtor
    )
endif ()

