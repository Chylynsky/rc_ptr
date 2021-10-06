#
# Copyright Borys Chyliński 2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.5)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY      https://github.com/catchorg/Catch2.git
    GIT_TAG             v2.13.7
)

FetchContent_MakeAvailable(Catch2)

set(TARGET rc_ptr-test)

set(REQUIRED_LIBS 
    Catch2::Catch2
    rc_ptr)

file(GLOB TEST_SRCS "unique.cpp" "test_main.cpp" "swap.cpp" "reset.cpp" "get.cpp" "get_deleter.cpp" "deleter.cpp" "constructor.cpp" "*.cpp")

add_executable(${TARGET} ${TEST_SRCS})

target_link_libraries(${TARGET} PRIVATE ${REQUIRED_LIBS})
