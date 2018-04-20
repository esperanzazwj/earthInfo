set(CEREAL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/library/cereal CACHE PATH "Cereal include")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Cereal
    REQUIRED_VARS CEREAL_INCLUDE_DIR
)

if (NOT TARGET Cereal)
    add_library(Cereal INTERFACE)
    target_include_directories(
        Cereal INTERFACE
        ${CEREAL_INCLUDE_DIR}
    )
endif ()

mark_as_advanced(CEREAL_INCLUDE_DIR)

