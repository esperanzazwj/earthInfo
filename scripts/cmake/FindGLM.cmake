set(GLM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/library/glm CACHE PATH "GLM include")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    GLM
    REQUIRED_VARS GLM_INCLUDE_DIR
)

if (NOT TARGET GLM)
    add_library(GLM INTERFACE)
    target_include_directories(
        GLM INTERFACE
        ${GLM_INCLUDE_DIR}
    )
endif ()

mark_as_advanced(GLM_INCLUDE_DIR)

