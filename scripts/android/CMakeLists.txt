cmake_minimum_required(VERSION 3.6)
set(GRADLE_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../..)

set(
    CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/scripts/cmake
)
include(build)

add_library(bootstrap SHARED ${GRADLE_DIR}/bootstrap.c)
target_link_libraries(
    bootstrap
    log
)

