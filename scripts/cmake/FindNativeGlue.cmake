if (ANDROID)
    set(NATIVE_GLUE_INCLUDE_DIR ${ANDROID_NDK}/sources/android/native_app_glue CACHE PATH "Android native app glue include")
    set(NATIVE_GLUE_SOURCE ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c CACHE PATH "Android native app glue source")

    if (NOT TARGET NativeGlue)
        add_library(NativeGlue STATIC ${NATIVE_GLUE_SOURCE})
        set_property(
            TARGET NativeGlue
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${NATIVE_GLUE_INCLUDE_DIR}
        )
    endif ()

    mark_as_advanced(NATIVE_GLUE_INCLUDE_DIR NATIVE_GLUE_SOURCE)
else ()
    add_library(NativeGlue INTERFACE)
endif ()

