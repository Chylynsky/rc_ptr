//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr.hpp"

TEST_CASE("swap, two nullptrs", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second{ nullptr };
    first.swap(second);
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("swap, nullptr and pointer", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second{ nullptr };
    first.swap(second);
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == raw);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
}

TEST_CASE("swap, double swap - nullptr and pointer", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second{ nullptr };
    first.swap(second);
    first.swap(second);
    REQUIRE(first.get() == raw);
    REQUIRE(first.use_count() == 1);
    REQUIRE(first.unique());
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("swap, valid pointers", "[rc_ptr]")
{
    auto raw_first  = new int{ 0 };
    auto raw_second = new int{ 6 };
    memory::rc_ptr<int> first{ raw_first };
    memory::rc_ptr<int> second{ raw_second };
    first.swap(second);
    REQUIRE(first.get() == raw_second);
    REQUIRE(first.use_count() == 1);
    REQUIRE(first.unique());
    REQUIRE(second.get() == raw_first);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
}

TEST_CASE("swap, double swap - valid pointers", "[rc_ptr]")
{
    auto raw_first  = new int{ 0 };
    auto raw_second = new int{ 6 };
    memory::rc_ptr<int> first{ raw_first };
    memory::rc_ptr<int> second{ raw_second };
    first.swap(second);
    first.swap(second);
    REQUIRE(first.get() == raw_first);
    REQUIRE(first.use_count() == 1);
    REQUIRE(first.unique());
    REQUIRE(second.get() == raw_second);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
}
