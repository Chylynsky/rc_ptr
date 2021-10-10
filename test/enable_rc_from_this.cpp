//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr.hpp"

class test : public memory::enable_rc_from_this<test>
{
};

TEST_CASE("enable_rc_from_this, rc_from_this", "[enable_rc_from_this]")
{
    auto first  = memory::rc_ptr<test>(new test{});
    auto second = first->rc_from_this();
    REQUIRE(first.use_count() == 2);
    REQUIRE(second.use_count() == 2);
}

TEST_CASE("enable_rc_from_this, weak_rc_from_this", "[enable_rc_from_this]")
{
    auto first  = memory::rc_ptr<test>(new test{});
    auto second = first->weak_rc_from_this();
    REQUIRE(first.use_count() == 1);
    REQUIRE(!second.expired());
}
