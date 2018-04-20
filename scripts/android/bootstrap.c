#include <android/native_activity.h>
#include <android/log.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ssEngine Bootstrap", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ssEngine Bootstrap", __VA_ARGS__)

#define LIB_PREFIX "/data/data/cn.edu.zju.cad.rendering.ss.engine/lib/"
typedef void entry_fn(ANativeActivity*, void*, size_t);

static void* load_lib(char const* path)
{
    LOGI("  Loading %s...", path);
    void* handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        LOGE("Failed to open library %s: %s", path, dlerror());
        exit(EXIT_FAILURE);
    }
    return handle;
}

void ANativeActivity_onCreate(ANativeActivity* app, void* ud, size_t udsize)
{
    LOGI("Loading libraries...");
    load_lib(LIB_PREFIX "libc++_shared.so");
    load_lib(LIB_PREFIX "libassimp.so");

    entry_fn* entry = dlsym(load_lib(LIB_PREFIX "libss-engine.so"), "ANativeActivity_onCreate");
    if (!entry) {
        LOGE("Failed to find entry point in main library: %s", dlerror());
        exit(EXIT_FAILURE);
    }

    LOGI("Switching control to entry...");
    entry(app, ud, udsize);
}

