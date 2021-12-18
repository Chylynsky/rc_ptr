//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include <functional>

#include "rc_ptr/rc_ptr.hpp"

TEST_CASE("rc_ptr, default constructor", "[constructor]")
{
    memory::rc_ptr<int> ptr;
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("rc_ptr, nullptr constructor", "[constructor]")
{
    memory::rc_ptr<int> ptr{ nullptr };
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("rc_ptr, nullptr constructor with deleter", "[constructor]")
{
    memory::rc_ptr<int, std::function<void(int*)>> ptr{
        nullptr,
        [](int* ptr) { delete ptr; },
    };
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("rc_ptr, pointer constructor", "[constructor]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> ptr{ raw };
    REQUIRE(ptr.get() == raw);
    REQUIRE(ptr.use_count() == 1);
    REQUIRE(ptr.unique());
}

TEST_CASE("rc_ptr, pointer constructor with deleter", "[constructor]")
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

TEST_CASE("rc_ptr, array constructor", "[constructor]")
{
    auto raw = new int[256];
    memory::rc_ptr<int[]> ptr{ raw };
    REQUIRE(ptr.get() == raw);
    REQUIRE(ptr.use_count() == 1);
    REQUIRE(ptr.unique());
}

TEST_CASE("rc_ptr, copy constructor", "[constructor]")
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

TEST_CASE("rc_ptr, copy constructor when nullptr", "[constructor]")
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

TEST_CASE("rc_ptr, move constructor", "[constructor]")
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

TEST_CASE("rc_ptr, move constructor when nullptr", "[constructor]")
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

TEST_CASE("weak_rc_ptr, default constructor", "[constructor]")
{
    memory::weak_rc_ptr<int> ptr;
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(ptr.expired());
}

TEST_CASE("weak_rc_ptr, nullptr constructor", "[constructor]")
{
    memory::weak_rc_ptr<int> ptr{ nullptr };
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(ptr.expired());
}

TEST_CASE("weak_rc_ptr, nullptr constructor with deleter", "[constructor]")
{
    memory::weak_rc_ptr<int, std::function<void(int*)>> ptr{
        nullptr,
    };
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(ptr.expired());
}

TEST_CASE("weak_rc_ptr, array constructor", "[constructor]")
{
    memory::weak_rc_ptr<int[]> ptr{};
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(ptr.expired());
}

TEST_CASE("weak_rc_ptr, copy constructor when nullptr", "[constructor]")
{
    memory::weak_rc_ptr<int> first{ nullptr };
    memory::weak_rc_ptr<int> second{ first };
    REQUIRE(first.use_count() == 0);
    REQUIRE(first.expired());
    REQUIRE(second.use_count() == 0);
    REQUIRE(second.expired());
}

TEST_CASE("weak_rc_ptr, move constructor when nullptr", "[constructor]")
{
    memory::weak_rc_ptr<int> first{ nullptr };
    memory::weak_rc_ptr<int> second{ std::move(first) };
    REQUIRE(first.use_count() == 0);
    REQUIRE(first.expired());
    REQUIRE(second.use_count() == 0);
    REQUIRE(second.expired());
}

TEST_CASE("mixed, weak_rc_ptr from rc_ptr", "[constructor]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::weak_rc_ptr<int> second{ first };
    REQUIRE(first.unique());
    REQUIRE(first.use_count() == 1);
    REQUIRE(second.use_count() == 1);
    REQUIRE(!second.expired());
}

TEST_CASE("mixed, rc_ptr from valid weak_rc_ptr", "[constructor]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::weak_rc_ptr<int> weak{ first };
    memory::rc_ptr<int> second{ weak };
    REQUIRE(!first.unique());
    REQUIRE(first.use_count() == 2);
    REQUIRE(!second.unique());
    REQUIRE(second.use_count() == 2);
    REQUIRE(!weak.expired());
    REQUIRE(weak.use_count() == 2);
}

TEST_CASE("mixed, rc_ptr from invalid weak_rc_ptr", "[constructor]")
{
    REQUIRE_THROWS_AS(std::invoke([]() {
        memory::weak_rc_ptr<int> first;
        memory::rc_ptr<int> second{ first };
    }),
        memory::bad_weak_rc_ptr);
}
