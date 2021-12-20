//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr/rc_ptr.hpp"

#include <map>

TEST_CASE("rc_ptr, owner_less comparator in std::map.", "[owner_before]")
{
    using namespace memory;
    std::map<rc_ptr<int>, int, owner_less<rc_ptr<int>>> map{
        memory::owner_less<rc_ptr<int>>{}
    };

    map[make_rc<int>(0)] = 0;
    map[make_rc<int>(1)] = 1;
    map[make_rc<int>(2)] = 2;

    for (const auto& [key, value] : map)
    {
        REQUIRE(*key == value);
    }
}

TEST_CASE("weak_rc_ptr, owner_less comparator in std::map.", "[owner_before]")
{
    using namespace memory;
    std::map<weak_rc_ptr<int>, int, owner_less<weak_rc_ptr<int>>> map{
        memory::owner_less<weak_rc_ptr<int>>{}
    };

    auto first  = make_rc<int>(0);
    auto second = make_rc<int>(1);
    auto third  = make_rc<int>(2);

    map[first]  = 0;
    map[second] = 1;
    map[third]  = 2;

    for (const auto& [key, value] : map)
    {
        REQUIRE(*(key.lock()) == value);
    }
}
