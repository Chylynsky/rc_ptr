#include "catch2/catch.hpp"

#include <cstdint>
#include <functional>

#include "rc_ptr.hpp"

template<typename T>
struct deleter_copy_constructible {
    deleter_copy_constructible()                                  = default;
    deleter_copy_constructible(const deleter_copy_constructible&) = default;
    void operator()(T* ptr)
    {
        delete ptr;
    }
};

template<typename T>
struct deleter_move_constructible {
    deleter_move_constructible()                                  = default;
    deleter_move_constructible(const deleter_move_constructible&) = delete;
    deleter_move_constructible(deleter_move_constructible&&)      = default;
    void operator()(T* ptr)
    {
        delete ptr;
    }
};

TEST_CASE("deleter, copy constructible", "[rc_ptr]")
{
    deleter_copy_constructible<int> deleter;
    memory::rc_ptr<int, deleter_copy_constructible<int>> ptr(nullptr, deleter);
}

TEST_CASE("deleter, move constructible", "[rc_ptr]")
{
    memory::rc_ptr<int, deleter_move_constructible<int>> ptr(nullptr, deleter_move_constructible<int>());
}

TEST_CASE("deleter, reference type", "[rc_ptr]")
{
    deleter_copy_constructible<int> deleter;
    memory::rc_ptr<int, deleter_copy_constructible<int>&> ptr(nullptr, deleter);
}

TEST_CASE("deleter, not called when nullptr", "[rc_ptr]")
{
    std::size_t times_called = 0;

    auto deleter = [&](int* ptr) mutable {
        delete ptr;
        ++times_called;
    };

    using deleter_type = decltype(deleter)&;
    static_assert(std::is_lvalue_reference_v<deleter_type>, "Deleter must be an lvalue reference.");

    {
        memory::rc_ptr<int, deleter_type> ptr(nullptr, deleter);
    }
    REQUIRE(times_called == 0);
}

TEST_CASE("deleter, one copy", "[rc_ptr]")
{
    using deleter_type = std::function<void(int*)>;
    static_assert(!std::is_lvalue_reference_v<deleter_type>, "Deleter must not be an lvalue reference.");
    static_assert(std::is_nothrow_move_constructible_v<deleter_type>, "Deleter must be nothrow move constructible");

    std::size_t times_called = 0;
    {
        memory::rc_ptr<int, deleter_type> ptr(new int{ 0 }, [&](int* ptr) {
            delete ptr;
            ++times_called;
        });
    }
    REQUIRE(times_called == 1);
}

TEST_CASE("deleter, two copies", "[rc_ptr]")
{
    using deleter_type = std::function<void(int*)>;
    static_assert(!std::is_lvalue_reference_v<deleter_type>, "Deleter must not be an lvalue reference.");
    static_assert(std::is_nothrow_move_constructible_v<deleter_type>, "Deleter must be nothrow move constructible");

    std::size_t times_called = 0;
    {
        memory::rc_ptr<int, deleter_type> first(new int{ 0 }, [&](int* ptr) {
            delete ptr;
            ++times_called;
        });

        {
            memory::rc_ptr<int, deleter_type> second = first;
        }
    }
    REQUIRE(times_called == 1);
}
