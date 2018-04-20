if (EMSCRIPTEN OR ANDROID)
    set(GLFW3_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/library/glfw/include CACHE PATH "GLFW3 include")

    if (NOT TARGET GLFW3)
        add_library(GLFW3 INTERFACE)
        set_property(
            TARGET GLFW3
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${GLFW3_INCLUDE_DIR}
        )
    endif ()

    mark_as_advanced(GLFW3_INCLUDE_DIR)
else ()
    find_library(
        GLFW3_LIBRARY
        NAMES glfw3dll glfw
        PATHS ${CMAKE_SOURCE_DIR}/library/glfw
    )
    find_path(
        GLFW3_INCLUDE_DIR
        NAMES GLFW/glfw3.h
        PATHS ${CMAKE_SOURCE_DIR}/library/glfw
        PATH_SUFFIXES include
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(
        GLFW3
        REQUIRED_VARS GLFW3_LIBRARY GLFW3_INCLUDE_DIR
    )

    if (NOT TARGET GLFW3)
        add_library(GLFW3 UNKNOWN IMPORTED)
        set_property(
            TARGET GLFW3
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${GLFW3_INCLUDE_DIR}
        )
        set_property(
            TARGET GLFW3
            PROPERTY IMPORTED_LOCATION ${GLFW3_LIBRARY}
        )
    endif ()

    mark_as_advanced(GLFW3_LIBRARY GLFW3_INCLUDE_DIR)
endif ()

