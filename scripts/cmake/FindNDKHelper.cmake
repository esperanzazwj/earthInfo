if (ANDROID)
    find_package(NativeGlue REQUIRED)
    set(src ${ANDROID_NDK}/sources/android/ndk_helper)

    if (NOT TARGET NDK_Helper_Core)
        add_library(NDK_Helper_Core STATIC
            ${src}/JNIHelper.cpp
            ${src}/JNIHelper.h
        )
        set_property(
            TARGET NDK_Helper_Core
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${src}
        )
        target_link_libraries(
            NDK_Helper_Core
            NativeGlue
        )
    endif ()

    if (NOT TARGET NDK_Helper_Math)
        add_library(NDK_Helper_Math STATIC
            ${src}/vecmath.cpp
            ${src}/vecmath.h
        )
        set_property(
            TARGET NDK_Helper_Math
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${src}
        )
        target_link_libraries(
            NDK_Helper_Math
            NDK_Helper_Core
        )
    endif ()

    if (NOT TARGET NDK_Helper_Gesture_Detector)
        add_library(NDK_Helper_Gesture_Detector STATIC
            ${src}/gestureDetector.cpp
            ${src}/gestureDetector.h
        )
        set_property(
            TARGET NDK_Helper_Gesture_Detector
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${src}
        )
        target_link_libraries(
            NDK_Helper_Gesture_Detector
            NDK_Helper_Core
            NDK_Helper_Math
            NativeGlue
        )
    endif ()
else ()
    add_library(NDK_Helper_Core INTERFACE)
    add_library(NDK_Helper_Math INTERFACE)
    add_library(NDK_Helper_Gesture_Detector INTERFACE)
endif ()

