cmake_minimum_required(VERSION 3.6)
project(testbed)
enable_language(CXX)
enable_language(C)
enable_language(ASM)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_STANDARD 11)

include_directories("/usr/local/include")
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
        src/dpdk/init.c
        src/dpdk/args.c)

set(DPDK_EXES
        src/dpdk/dpdk_server.c
        ./src/dpdk/dpck_client.cpp)

add_compile_options("-march=native")
add_definitions(-DALLOW_EXPERIMENTAL_API)
set(DPDK_DEPS
        -lrte_node -lrte_graph -lrte_bpf -lrte_flow_classify -lrte_pipeline -lrte_table -lrte_port -lrte_fib
        -lrte_ipsec -lrte_vhost -lrte_stack -lrte_security -lrte_sched -lrte_reorder -lrte_rib -lrte_regexdev
        -lrte_rawdev -lrte_pdump -lrte_power -lrte_member -lrte_lpm -lrte_latencystats -lrte_kni -lrte_jobstats
        -lrte_ip_frag -lrte_gso -lrte_gro -lrte_eventdev -lrte_efd -lrte_distributor -lrte_cryptodev -lrte_compressdev
        -lrte_cfgfile -lrte_bitratestats -lrte_bbdev -lrte_acl -lrte_timer -lrte_hash -lrte_metrics -lrte_cmdline
        -lrte_pci -lrte_ethdev -lrte_meter -lrte_net -lrte_mbuf -lrte_mempool -lrte_rcu -lrte_ring -lrte_eal
        -lrte_telemetry -lrte_kvargs)

if (DPDK_VERSION)
    if (${DPDK_VERSION} GREATER 16)
        add_definitions(-DDPDK_VERSION=20)
    else ()
        add_definitions(-DDPDK_VERSION=16)
    endif ()
else ()
    add_definitions(-DDPDK_VERSION=20)
endif ()

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
        target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} -L../libs ${DPDK_DEPS} -lpthread -levent -ljemalloc-osx)
    else ()
        target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} -L./libs ${DPDK_DEPS} ${NUMA_DEP} -lpthread -levent -ljemalloc)
    endif ()
endforeach (sourcefile ${DPDK_TESTS})

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_subdirectory(src/dist/mica2)
endif ()

add_subdirectory(src/dist/anna)
