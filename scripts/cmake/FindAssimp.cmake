if (EMSCRIPTEN)
    set(ASSIMP_LIBRARY ${CMAKE_SOURCE_DIR}/library/assimp/assimp.bc CACHE FILEPATH "Assimp library")
    set(ASSIMP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/library/assimp/include CACHE PATH "Assimp include")
elseif (ANDROID)
    set(ASSIMP_LIBRARY ${CMAKE_SOURCE_DIR}/bin/android/${ANDROID_ABI}/libassimp.so CACHE FILEPATH "Assimp library")
    set(ASSIMP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/library/assimp/include-windows CACHE PATH "Assimp include")
else ()
    if (MSVC)
        set(include-dir-suffix "-windows")
    else ()
        set(include-dir-suffix "")
    endif ()

    find_library(
        ASSIMP_LIBRARY
        NAMES assimp-vc140-mt assimp
        PATHS ${CMAKE_SOURCE_DIR}/library/assimp
    )
    find_path(
        ASSIMP_INCLUDE_DIR
        NAMES assimp/mesh.h
        PATHS ${CMAKE_SOURCE_DIR}/library/assimp
        PATH_SUFFIXES "include${include-dir-suffix}"
    )
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Assimp
    REQUIRED_VARS ASSIMP_LIBRARY ASSIMP_INCLUDE_DIR
)

if (NOT TARGET Assimp)
    add_library(Assimp UNKNOWN IMPORTED)
    set_property(
        TARGET Assimp
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${ASSIMP_INCLUDE_DIR}
    )
    set_property(
        TARGET Assimp
        PROPERTY IMPORTED_LOCATION ${ASSIMP_LIBRARY}
    )
endif ()

mark_as_advanced(ASSIMP_LIBRARY ASSIMP_INCLUDE_DIR)

