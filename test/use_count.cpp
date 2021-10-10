//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#include "catch2/catch.hpp"

#include "rc_ptr.hpp"

TEST_CASE("rc_ptr, use_count after default construction", "[use_count]")
{
    memory::rc_ptr<int> first;
    REQUIRE(first.use_count() == 0);
}

TEST_CASE("rc_ptr, use_count after nullptr construction", "[use_count]")
{
    memory::rc_ptr<int> first{ nullptr };
    REQUIRE(first.use_count() == 0);
}

TEST_CASE("rc_ptr, use_count after pointer construction", "[use_count]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    REQUIRE(first.use_count() == 1);
}

TEST_CASE("rc_ptr, use_count after two copies", "[use_count]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::rc_ptr<int> second = first;
    REQUIRE(first.use_count() == 2);
    REQUIRE(second.use_count() == 2);
}

TEST_CASE("rc_ptr, use_count after after scope", "[use_count]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    {
        memory::rc_ptr<int> second = first;
    }
    REQUIRE(first.use_count() == 1);
}

TEST_CASE("rc_ptr, use_count after after move", "[use_count]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::rc_ptr<int> second = std::move(first);
    REQUIRE(first.use_count() == 0);
    REQUIRE(second.use_count() == 1);
}

TEST_CASE("weak_rc_ptr, use_count after default construction", "[use_count]")
{
    memory::rc_ptr<int> first;
    memory::weak_rc_ptr<int> second = first;
    REQUIRE(second.use_count() == 0);
}

TEST_CASE("weak_rc_ptr, use_count after nullptr construction", "[use_count]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::weak_rc_ptr<int> second = first;
    REQUIRE(second.use_count() == 0);
}

TEST_CASE("weak_rc_ptr, use_count after pointer construction", "[use_count]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::weak_rc_ptr<int> second = first;
    REQUIRE(second.use_count() == 1);
}

TEST_CASE("weak_rc_ptr, use_count after two copies", "[use_count]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::rc_ptr<int> second     = first;
    memory::weak_rc_ptr<int> third = first;
    REQUIRE(third.use_count() == 2);
}

TEST_CASE("weak_rc_ptr, use_count after after scope", "[use_count]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::weak_rc_ptr<int> second = first;
    {
        memory::rc_ptr<int> third = first;
    }
    REQUIRE(second.use_count() == 1);
}

TEST_CASE("weak_rc_ptr, use_count after after move", "[use_count]")
{
    memory::rc_ptr<int> first(new int{ 0 });
    memory::rc_ptr<int> second     = std::move(first);
    memory::weak_rc_ptr<int> third = second;
    REQUIRE(third.use_count() == 1);
}
