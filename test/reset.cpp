#include "catch2/catch.hpp"

#include "rc_ptr.hpp"

TEST_CASE("reset, default construction", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr;
    ptr.reset();
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("reset, nullptr construction", "[rc_ptr]")
{
    memory::rc_ptr<int> ptr{ nullptr };
    ptr.reset();
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}

TEST_CASE("reset, copy construction", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second{ first };
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("reset, move construction", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second{ std::move(first) };
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("reset, after copy assignment", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second = first;
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("reset, after move assignment", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second = std::move(first);
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("reset, after swap", "[rc_ptr]")
{
    memory::rc_ptr<int> first{ new int{ 0 } };
    memory::rc_ptr<int> second{ new int{ 6 } };
    first.swap(second);
    first.reset();
    REQUIRE(first.get() == nullptr);
    REQUIRE(first.use_count() == 0);
    REQUIRE(!first.unique());
    REQUIRE(second.get() != nullptr);
    REQUIRE(second.use_count() == 1);
    REQUIRE(second.unique());
    second.reset();
    REQUIRE(second.get() == nullptr);
    REQUIRE(second.use_count() == 0);
    REQUIRE(!second.unique());
}

TEST_CASE("reset, custom deleter", "[rc_ptr]")
{
    auto deleter = [](int* ptr) {
        delete ptr;
    };

    using deleter_type = decltype(deleter)&;

    memory::rc_ptr<int, deleter_type> ptr{ new int{ 0 }, deleter };

    ptr.reset();
    REQUIRE(ptr.get() == nullptr);
    REQUIRE(ptr.use_count() == 0);
    REQUIRE(!ptr.unique());
}
