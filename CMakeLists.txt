#
# Copyright Borys Chyliński 2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.5)

project(rc_ptr-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET rc_ptr)

set(RC_PTR_BUILD_TESTS ON)

add_library(${TARGET} INTERFACE)

target_include_directories(${TARGET} INTERFACE include)

if (RC_PTR_BUILD_TESTS)
    add_subdirectory(test rc_ptr-test)
endif()
