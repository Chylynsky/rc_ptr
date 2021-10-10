//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr.hpp"

TEST_CASE("weak_rc_ptr, lock on default constructed", "[lock]")
{
    REQUIRE_THROWS_AS(std::invoke([]() {
        memory::weak_rc_ptr<int> first;
        first.lock();
    }),
        memory::bad_weak_rc_ptr);
}

TEST_CASE("weak_rc_ptr, lock on valid rc_ptr constructed", "[lock]")
{
    int* raw = new int{ 6 };
    memory::rc_ptr<int> first{ raw };
    memory::weak_rc_ptr<int> second{ first };
    auto third = first.lock();
    REQUIRE(third.use_count() == 2);
    REQUIRE(third.get() == raw);
}

TEST_CASE("weak_rc_ptr, lock on invalid rc_ptr constructed", "[lock]")
{
    REQUIRE_THROWS_AS(std::invoke([]() {
        memory::rc_ptr<int> first{ nullptr };
        memory::weak_rc_ptr<int> second{ first };
        second.lock();
    }),
        memory::bad_weak_rc_ptr);
}

TEST_CASE("weak_rc_ptr, lock on expired", "[lock]")
{
    REQUIRE_THROWS_AS(std::invoke([]() {
        memory::weak_rc_ptr<int> first;
        {
            memory::rc_ptr<int> second{ new int{ 0 } };
            first = second;
        }
    }),
        memory::bad_weak_rc_ptr);
}
