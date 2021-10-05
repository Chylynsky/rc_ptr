#include "catch2/catch.hpp"

#include <functional>

#include "rc_ptr.hpp"

TEST_CASE("assignment, nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr = nullptr;
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("assignment, copy", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second = first;
    REQUIRE(first.get() == raw);
    REQUIRE(first.use_count() == 2);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == raw);
    REQUIRE(second.use_count() == 2);
    REQUIRE(!second.unique());
}

TEST_CASE("assignment, copy when nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second = first;
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("assignment, move", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    memory::rc_ptr<int> first{ raw };
    memory::rc_ptr<int> second = std::move(first);
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == raw);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
}

TEST_CASE("assignment, move when nullptr", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ nullptr };
    memory::rc_ptr<int> second = std::move(first);
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("assignment, nullptr with deleter", "[rc_ptr]")
{
    auto ptr = memory::rc_ptr<int, std::function<void(int*)>>{
        nullptr,
        [](int* ptr) { delete ptr; },
    };
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("assignment, pointer with deleter", "[rc_ptr]")
{
    auto raw = new int{ 0 };
    auto ptr = memory::rc_ptr<int, std::function<void(int*)>>{
        raw,
        [](int* ptr) { delete ptr; },
    };
    REQUIRE(ptr.get() != nullptr);
    REQUIRE(ptr.use_count() == 1);
    REQUIRE(ptr.unique());
}