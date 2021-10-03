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

file(GLOB TEST_SRCS "*.cpp")

add_executable(${TARGET} ${TEST_SRCS})

target_link_libraries(${TARGET} PRIVATE ${REQUIRED_LIBS})
