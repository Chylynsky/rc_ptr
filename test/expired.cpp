//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr/rc_ptr.hpp"

TEST_CASE("weak_rc_ptr, expired when default constructed", "[expired]")
{
    memory::weak_rc_ptr<int> ptr;
    REQUIRE(ptr.expired());
}

TEST_CASE("weak_rc_ptr, expired when constructed from invalid rc_ptr", "[expired]")
{
    memory::rc_ptr<int> first;
    memory::weak_rc_ptr<int> second{ first };
    REQUIRE(second.expired());
}

TEST_CASE("weak_rc_ptr, not expired when constructed from valid rc_ptr", "[expired]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::weak_rc_ptr<int> second{ first };
    REQUIRE(!second.expired());
}

TEST_CASE("weak_rc_ptr, expired when last rc_ptr is destroyed", "[expired]")
{
    memory::weak_rc_ptr<int> first;
    {
        memory::rc_ptr<int> second{ new int{ 0 } };
        first = second;
    }
    REQUIRE(first.expired());
}
