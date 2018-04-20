
find_library(
    GLEW_WINDOWS_LIBRARY
    NAMES glew32
    PATHS ${CMAKE_SOURCE_DIR}/library/glew
)
find_path(
    GLEW_WINDOWS_INCLUDE_DIR
    NAMES GL/glew.h
    PATHS ${CMAKE_SOURCE_DIR}/library/glew
    PATH_SUFFIXES include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    GLEW_WINDOWS
    REQUIRED_VARS GLEW_WINDOWS_LIBRARY GLEW_WINDOWS_INCLUDE_DIR
)

if (NOT TARGET GLEW::GLEW)
    add_library(GLEW::GLEW UNKNOWN IMPORTED)
    set_property(
        TARGET GLEW::GLEW
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${GLEW_WINDOWS_INCLUDE_DIR}
    )
    set_property(
        TARGET GLEW::GLEW
        PROPERTY IMPORTED_LOCATION ${GLEW_WINDOWS_LIBRARY}
    )
endif ()

mark_as_advanced(GLEW_WINDOWS_LIBRARY GLEW_WINDOWS_INCLUDE_DIR)

