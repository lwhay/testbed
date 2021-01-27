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

set(NUMA_DEP -lnuma)

set(COM_LIBS
        )

set(COM_EXES
        ./src/com/socket_server.cpp
        ./src/com/socket_client.cpp)

set(COM_TESTS ${COM_EXES})
foreach (sourcefile ${COM_TESTS})
    get_filename_component(exename ${sourcefile} NAME_WE)
    add_executable(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${sourcefile} ${COM_LIBS})
    set_target_properties(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX}
            PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_MINRELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO 1
            OUTPUT_NAME ${exename}${ARTIFACT_SUFFIX}
            )
    if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
        target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} -L../libs -lpthread -levent -ljemalloc-osx)
    else ()
        target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} -L./libs ${NUMA_DEP} -lpthread -levent -ljemalloc)
    endif ()
endforeach (sourcefile ${COM_TESTS})

set(DPDK_LIBS
        ./src/dpdk/init.cpp)

set(DPDK_EXES
        #        ./src/dpdk/dpdk_server.cpp
        #        ./src/dpdk/dpck_client.cpp
        )

set(DPDK_TESTS ${DPDK_EXES})
foreach (sourcefile ${DPDK_TESTS})
    get_filename_component(exename ${sourcefile} NAME_WE)
    add_executable(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${sourcefile} ${DPDK_LIBS})
    set_target_properties(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX}
            PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_MINRELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO 1
            OUTPUT_NAME ${exename}${ARTIFACT_SUFFIX}
            )
    if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
        target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} -L../libs -lpthread -levent -ljemalloc-osx)
    else ()
        target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} -L./libs ${NUMA_DEP} -lpthread -levent -ljemalloc)
    endif ()
endforeach (sourcefile ${DPDK_TESTS})

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_subdirectory(src/dist/mica2)
endif ()

add_subdirectory(src/dist/anna)
