//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr.hpp"

TEST_CASE("unique, default construction", "[rc_ptr]")
{
    memory::rc_ptr<int> first;
    REQUIRE(!first.unique());
}

TEST_CASE("unique, nullptr construction", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    REQUIRE(!first.unique());
}

TEST_CASE("unique, pointer construction", "[rc_ptr]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    REQUIRE(first.unique());
}

TEST_CASE("unique, two copies", "[rc_ptr]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::rc_ptr<int> second = first;
    REQUIRE(!first.unique());
    REQUIRE(!second.unique());
}

TEST_CASE("unique, after scope", "[rc_ptr]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    {
        memory::rc_ptr<int> second = first;
    }
    REQUIRE(first.unique());
}

TEST_CASE("unique, after move", "[rc_ptr]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::rc_ptr<int> second = std::move(first);
    REQUIRE(!first.unique());
    REQUIRE(second.unique());
}
