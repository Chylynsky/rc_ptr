//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr/rc_ptr.hpp"

TEST_CASE("make_rc, int", "[make_rc]")
{
    memory::rc_ptr ptr = memory::make_rc<int>(10);
}
