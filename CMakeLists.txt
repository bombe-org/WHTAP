cmake_minimum_required(VERSION 2.8)
project(rundb)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-pg -g ${CMAKE_CXX_FLAGS}")
SET(LINK_FLAGS "-pg ${LINK_FLAGS}")
add_definitions(-D NOGRAPHITE=1)

set(SOURCE_FILES
        benchmarks/test_txn.cpp
        benchmarks/test_wl.cpp
        benchmarks/tpcc_helper.cpp
        benchmarks/tpcc_query.cpp
        benchmarks/tpcc_txn.cpp
        benchmarks/tpcc_wl.cpp
        benchmarks/ycsb_query.cpp
        benchmarks/ycsb_txn.cpp
        benchmarks/ycsb_wl.cpp
        concurrency_control/dl_detect.cpp
        concurrency_control/hekaton.cpp
        concurrency_control/occ.cpp
        concurrency_control/plock.cpp
        concurrency_control/row_hekaton.cpp
        concurrency_control/row_lock.cpp
        concurrency_control/row_mvcc.cpp
        concurrency_control/row_occ.cpp
        concurrency_control/row_silo.cpp
        concurrency_control/row_tictoc.cpp
        concurrency_control/row_ts.cpp
        concurrency_control/row_vll.cpp
        concurrency_control/silo.cpp
        concurrency_control/tictoc.cpp
        storage/catalog.cpp
        storage/index_btree.cpp
        storage/index_hash.cpp
        storage/row.cpp
        storage/table.cpp
        system/global.cpp
        system/helper.cpp
        system/main.cpp
        system/manager.cpp
        system/mem_alloc.cpp
        system/parser.cpp
        system/query.cpp
        system/stats.cpp
        system/thread.cpp
        system/txn.cpp
        system/wl.cpp
        config.cpp)

add_executable(rundb ${SOURCE_FILES})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ./benchmarks/ ./concurrency_control/ ./storage/ ./system/)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})


find_package(Threads)
target_link_libraries(rundb ${CMAKE_THREAD_LIBS_INIT})
