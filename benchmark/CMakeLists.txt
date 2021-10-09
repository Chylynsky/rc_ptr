#
# Copyright Borys Chyliński 2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.5)

include(FetchContent)

set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.6.0
)

FetchContent_MakeAvailable(benchmark)

set(TARGET rc_ptr-benchmark)

set(REQUIRED_LIBS 
    benchmark::benchmark_main
    rc_ptr)

set(TEST_SRCS "benchmark.cpp")

add_executable(${TARGET} ${TEST_SRCS})

target_link_libraries(${TARGET} PRIVATE ${REQUIRED_LIBS})
