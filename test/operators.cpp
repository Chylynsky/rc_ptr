#include "catch2/catch.hpp"

#include <utility>

#include "rc_ptr.hpp"

TEST_CASE("operators, bool conversion when nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr{ nullptr };
    REQUIRE(!ptr);
}

TEST_CASE("operators, bool conversion when valid", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr{ new int{ 0 } };
    REQUIRE(ptr);
}

TEST_CASE("operators, operator*", "[rc_ptr]")
{
    constexpr int value = 13;
    memory::rc_ptr<int> ptr{ new int{ value } };
    REQUIRE(*ptr == value);
}

TEST_CASE("operators, operator->", "[rc_ptr]")
{
    constexpr std::pair<int, int> value{ 26, 34 };
    memory::rc_ptr<std::pair<int, int>> ptr{ new std::pair<int, int>{ value } };
    REQUIRE(ptr->first == value.first);
    REQUIRE(ptr->second == value.second);
}
