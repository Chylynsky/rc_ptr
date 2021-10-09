//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr.hpp"

TEST_CASE("rc_ptr, reset after default construction", "[reset]")
{
    memory::rc_ptr<int> ptr;
    ptr.reset();
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("rc_ptr, reset after nullptr construction", "[reset]")
{
    memory::rc_ptr<int> ptr{ nullptr };
    ptr.reset();
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("rc_ptr, reset after copy construction", "[reset]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second{ first };
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("rc_ptr, reset after move construction", "[reset]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second{ std::move(first) };
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("rc_ptr, reset after after copy assignment", "[reset]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second = first;
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("rc_ptr, reset after after move assignment", "[reset]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second = std::move(first);
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("rc_ptr, reset after after swap", "[reset]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second{ new int{ 6 } };
    first.swap(second);
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("rc_ptr, reset when constructed with custom deleter", "[reset]")
{
    auto deleter = [](int* ptr) {
        delete ptr;
    };

    using deleter_type = decltype(deleter)&;

    memory::rc_ptr<int, deleter_type> ptr{ new int{ 0 }, deleter };

    ptr.reset();
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}
