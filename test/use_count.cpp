//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr.hpp"

TEST_CASE("use_count, default construction", "[rc_ptr]")
{
    memory::rc_ptr<int> first;
    REQUIRE(first.use_count() == 0);
}

TEST_CASE("use_count, nullptr construction", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    REQUIRE(first.use_count() == 0);
}

TEST_CASE("use_count, pointer construction", "[rc_ptr]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    REQUIRE(first.use_count() == 1);
}

TEST_CASE("use_count, two copies", "[rc_ptr]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::rc_ptr<int> second = first;
    REQUIRE(first.use_count() == 2);
    REQUIRE(second.use_count() == 2);
}

TEST_CASE("use_count, after scope", "[rc_ptr]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    {
        memory::rc_ptr<int> second = first;
    }
    REQUIRE(first.use_count() == 1);
}

TEST_CASE("use_count, after move", "[rc_ptr]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::rc_ptr<int> second = std::move(first);
    REQUIRE(first.use_count() == 0);
    REQUIRE(second.use_count() == 1);
}
