//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr/rc_ptr.hpp"

TEST_CASE("rc_tr, indexed access", "[array]")
{
    memory::rc_ptr<int[]> ptr{ new int[4] };

    for (std::size_t i = 0; i < 4; ++i)
    {
        ptr[i] = i;
    }

    REQUIRE(ptr[0] == 0);
    REQUIRE(ptr[1] == 1);
    REQUIRE(ptr[2] == 2);
    REQUIRE(ptr[3] == 3);
}
