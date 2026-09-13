cmake_minimum_required(VERSION 3.18.1)
project("Soggy")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES "*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "\\.cache/")
list(FILTER SOURCES EXCLUDE REGEX "/build/")
list(FILTER SOURCES EXCLUDE REGEX "/output/")

add_library(Soggy SHARED ${SOURCES})


target_include_directories(Soggy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Soggy
    ${CMAKE_CURRENT_SOURCE_DIR}/PVZ2
    ${CMAKE_CURRENT_SOURCE_DIR}/../And64Inline
)

target_link_libraries(Soggy
    log
    dl
)
