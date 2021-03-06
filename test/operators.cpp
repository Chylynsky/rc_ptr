//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include <utility>

#include "rc_ptr/rc_ptr.hpp"

TEST_CASE("rc_ptr, bool conversion when nullptr", "[operators]")
{
    memory::rc_ptr<int> ptr{ nullptr };
    REQUIRE(!ptr);
}

TEST_CASE("rc_ptr, bool conversion when valid", "[operators]")
{
    memory::rc_ptr<int> ptr{ new int{ 0 } };
    REQUIRE(ptr);
}

TEST_CASE("rc_ptr, operator*", "[operators]")
{
    constexpr int       value = 13;
    memory::rc_ptr<int> ptr{ new int{ value } };
    REQUIRE(*ptr == value);
}

TEST_CASE("rc_ptr, operator->", "[operators]")
{
    constexpr std::pair<int, int>       value{ 26, 34 };
    memory::rc_ptr<std::pair<int, int>> ptr{ new std::pair<int, int>{ value } };
    REQUIRE(ptr->first == value.first);
    REQUIRE(ptr->second == value.second);
}
