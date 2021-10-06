//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include <functional>

#include "rc_ptr.hpp"

TEST_CASE("get, default", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr;
    REQUIRE(ptr.get() == nullptr);
}

TEST_CASE("get, nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr{ nullptr };
    REQUIRE(ptr.get() == nullptr);
}

TEST_CASE("get, nullptr with deleter", "[rc_ptr]")
{
    memory::rc_ptr<int, std::function<void(int*)>> ptr{
        nullptr,
        [](int* ptr) { delete ptr; },
    };
    REQUIRE(ptr.get() == nullptr);
}

TEST_CASE("get, pointer", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> ptr{ raw };
    REQUIRE(ptr.get() == raw);
}

TEST_CASE("get, pointer with deleter", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int, std::function<void(int*)>> ptr{
        raw,
        [](int* ptr) { delete ptr; },
    };
    REQUIRE(ptr.get() == raw);
}

TEST_CASE("get, copy", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second{ first };
    REQUIRE(first.get() == raw);
    REQUIRE(second.get() == raw);
}

TEST_CASE("get, copy when nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second{ first };
    REQUIRE(first.get() == nullptr);
    REQUIRE(second.get() == nullptr);
}

TEST_CASE("get, move", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second{ std::move(first) };
    REQUIRE(first.get() == nullptr);
    REQUIRE(second.get() == raw);
}

TEST_CASE("get, move when nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second{ std::move(first) };
    REQUIRE(first.get() == nullptr);
}
