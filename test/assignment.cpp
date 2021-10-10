//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include <functional>

#include "rc_ptr.hpp"

TEST_CASE("rc_ptr, nullptr assignment", "[assignment]")
{
    memory::rc_ptr<int> ptr;
    ptr = nullptr;
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("rc_ptr, copy assignment", "[assignment]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second;
    second = first;
    REQUIRE(first.get() == raw);
    REQUIRE(first.use_count() == 2);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == raw);
    REQUIRE(second.use_count() == 2);
    REQUIRE(!second.unique());
}

TEST_CASE("rc_ptr, copy assignment when nullptr", "[assignment]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second;
    second = first;
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("rc_ptr, move assignment", "[assignment]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second;
    second = std::move(first);
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == raw);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
}

TEST_CASE("rc_ptr, move assignment when nullptr", "[assignment]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second;
    second = std::move(first);
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("rc_ptr, nullptr assignment with deleter", "[assignment]")
{
    memory::rc_ptr<int, std::function<void(int*)>> ptr;
    ptr = memory::rc_ptr<int, std::function<void(int*)>>{
        nullptr,
        [](int* ptr) { delete ptr; },
    };
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("rc_ptr, pointer assignment with deleter", "[assignment]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int, std::function<void(int*)>> ptr;
    ptr = memory::rc_ptr<int, std::function<void(int*)>>{
        raw,
        [](int* ptr) { delete ptr; },
    };
    REQUIRE(ptr.get() != nullptr);
    REQUIRE(ptr.use_count() == 1);
    REQUIRE(ptr.unique());
}

TEST_CASE("rc_ptr, assignment of weak_rc_ptr when valid", "[assignment]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::weak_rc_ptr<int> second{ first };
    memory::rc_ptr<int> third;
    third = second;
    REQUIRE(first.get() == raw);
    REQUIRE(first.use_count() == 2);
    REQUIRE(!first.unique());
    REQUIRE(!second.expired());
    REQUIRE(second.use_count() == 2);
    REQUIRE(third.get() == raw);
    REQUIRE(third.use_count() == 2);
    REQUIRE(!third.unique());
}

TEST_CASE("rc_ptr, assignment of weak_rc_ptr when invalid", "[assignment]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::weak_rc_ptr<int> second{ first };
    memory::rc_ptr<int> third;
    REQUIRE_THROWS_AS(std::invoke([&]() { third = second; }), memory::bad_weak_rc_ptr);
}

TEST_CASE("weak_rc_ptr, copy assignment when default constructed", "[assignment]")
{
    memory::weak_rc_ptr<int> first;
    memory::weak_rc_ptr<int> second;
    second = first;
    REQUIRE(first.expired());
    REQUIRE(first.use_count() == 0);
    REQUIRE(second.expired());
    REQUIRE(second.use_count() == 0);
}

TEST_CASE("weak_rc_ptr, copy assignment when valid", "[assignment]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::weak_rc_ptr<int> second{ first };
    memory::weak_rc_ptr<int> third;
    third = second;
    REQUIRE(!second.expired());
    REQUIRE(second.use_count() == 1);
    REQUIRE(!third.expired());
    REQUIRE(third.use_count() == 1);
}

TEST_CASE("weak_rc_ptr, move assignment when valid", "[assignment]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::weak_rc_ptr<int> second{ first };
    memory::weak_rc_ptr<int> third;
    third = std::move(second);
    REQUIRE(second.expired());
    REQUIRE(second.use_count() == 0);
    REQUIRE(!third.expired());
    REQUIRE(third.use_count() == 1);
}

TEST_CASE("weak_rc_ptr, assignment of valid rc_ptr", "[assignment]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::weak_rc_ptr<int> second;
    second = first;
    REQUIRE(!second.expired());
    REQUIRE(second.use_count() == 1);
}

TEST_CASE("weak_rc_ptr, assignment of invalid rc_ptr", "[assignment]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::weak_rc_ptr<int> second;
    second = first;
    REQUIRE(second.expired());
    REQUIRE(second.use_count() == 0);
}
