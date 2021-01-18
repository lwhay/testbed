cmake_minimum_required(VERSION 3.6)
project(testbed)
enable_language(CXX)
enable_language(C)
enable_language(ASM)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_STANDARD 11)

include_directories("./src")
include_directories("./src/utils")
include_directories("./src/dist")
include_directories("./src/dist/memcached")

add_executable(mc_mt ./src/dist/memcached/mc_mt.cpp)

target_link_libraries(mc_mt pthread)

add_executable(mc_mtr ./src/dist/memcached/mc_mtr.cpp)

target_link_libraries(mc_mtr pthread memcached)

add_executable(mc_mt_ycsb ./src/dist/memcached/mc_mt_ycsb.cpp)

target_link_libraries(mc_mt_ycsb pthread memcached)

add_executable(huhu_test ./src/dist/memcached/test.cpp)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_subdirectory(src/dist/mica2)
endif ()

add_subdirectory(src/dist/anna)