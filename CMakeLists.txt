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
