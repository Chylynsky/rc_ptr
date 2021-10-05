#include "catch2/catch.hpp"

#include <functional>

#include "rc_ptr.hpp"

TEST_CASE("constructor, default", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr;
}

TEST_CASE("constructor, nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr{ nullptr };
}

TEST_CASE("constructor, nullptr with deleter", "[rc_ptr]")
{
    memory::rc_ptr<int, std::function<void(int*)>> ptr{
        nullptr,
        [](int* ptr) { delete ptr; },
    };
}

TEST_CASE("constructor, pointer", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> ptr{ raw };
}

TEST_CASE("constructor, pointer with deleter", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int, std::function<void(int*)>> ptr{
        raw,
        [](int* ptr) { delete ptr; },
    };
}

TEST_CASE("constructor, copy", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second{ first };
}

TEST_CASE("constructor, copy when nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second{ first };
}

TEST_CASE("constructor, move", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second{ std::move(first) };
}

TEST_CASE("constructor, move when nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second{ std::move(first) };
}
