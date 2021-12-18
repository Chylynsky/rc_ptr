//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr/rc_ptr.hpp"

TEST_CASE("rc_ptr, get default deleter", "[get_deleter]")
{
    memory::rc_ptr<int> ptr{ new int{ 0 } };
    auto deleter = ptr.get_deleter();
    auto raw     = new int{ 0 };
    deleter(raw);
}

TEST_CASE("rc_ptr, get default deleter for array", "[get_deleter]")
{
    memory::rc_ptr<int[]> ptr{ new int[256] };
    auto deleter = ptr.get_deleter();
    auto raw     = new int[256];
    deleter(raw);
}

TEST_CASE("rc_ptr, get custom deleter", "[get_deleter]")
{
    std::size_t times_called = 0;

    auto deleter = [&](int* ptr) mutable {
        delete ptr;
        ++times_called;
    };

    using deleter_type = decltype(deleter)&;

    memory::rc_ptr<int, deleter_type> ptr(new int{ 0 }, deleter);

    auto _deleter = ptr.get_deleter();
    _deleter(new int{ 0 });
    REQUIRE(times_called == 1);
}
