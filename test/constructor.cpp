//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include <functional>

#include "rc_ptr.hpp"

TEST_CASE("constructor, default", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr;
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("constructor, nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr{ nullptr };
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("constructor, nullptr with deleter", "[rc_ptr]")
{
    memory::rc_ptr<int, std::function<void(int*)>> ptr{
        nullptr,
        [](int* ptr) { delete ptr; },
    };
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("constructor, pointer", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> ptr{ raw };
    REQUIRE(ptr.get() == raw);
    REQUIRE(ptr.use_count() == 1);
    REQUIRE(ptr.unique());
}

TEST_CASE("constructor, pointer with deleter", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int, std::function<void(int*)>> ptr{
        raw,
        [](int* ptr) { delete ptr; },
    };
    REQUIRE(ptr.get() == raw);
    REQUIRE(ptr.use_count() == 1);
    REQUIRE(ptr.unique());
}

TEST_CASE("constructor, array", "[rc_ptr]")
{
    auto raw = new int[256];
    memory::rc_ptr<int[]> ptr{ raw };
    REQUIRE(ptr.get() == raw);
    REQUIRE(ptr.use_count() == 1);
    REQUIRE(ptr.unique());
}

TEST_CASE("constructor, copy", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second{ first };
    REQUIRE(first.get() == raw);
    REQUIRE(first.use_count() == 2);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == raw);
    REQUIRE(second.use_count() == 2);
    REQUIRE(!second.unique());
}

TEST_CASE("constructor, copy when nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second{ first };
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("constructor, move", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second{ std::move(first) };
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == raw);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
}

TEST_CASE("constructor, move when nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second{ std::move(first) };
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}
