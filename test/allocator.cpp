//
// Copyright Borys Chyliński 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

#if (__has_include(<memory_resource>))

#include "catch2/catch.hpp"

#include "rc_ptr/rc_ptr.hpp"

#include <array>
#include <memory>
#include <memory_resource>

TEST_CASE("rc_ptr, construct with pmr allocator", "[allocator]")
{
    std::array<std::byte, 128>          buffer;
    std::pmr::monotonic_buffer_resource resource{
        buffer.data(),
        buffer.size(),
    };
    std::pmr::polymorphic_allocator<int> alloc{ &resource };

    int* raw = alloc.allocate(1);

    auto deleter = [=](int* ptr) mutable {
        alloc.deallocate(ptr, 1);
    };

    memory::rc_ptr<int, decltype(deleter), std::pmr::polymorphic_allocator<int>>
        ptr{ raw, deleter, alloc };

    REQUIRE(ptr);
    REQUIRE(ptr.unique());
    REQUIRE(ptr.use_count() == 1);
}

TEST_CASE("rc_ptr, construct array with pmr allocator", "[allocator]")
{
    std::pmr::polymorphic_allocator<int> alloc{};

    int* raw = alloc.allocate(100);

    auto deleter = [=](int* ptr) mutable {
        alloc.deallocate(ptr, 100);
    };

    memory::
        rc_ptr<int[], decltype(deleter), std::pmr::polymorphic_allocator<int>>
            ptr{ raw, deleter, alloc };

    REQUIRE(ptr);
    REQUIRE(ptr.unique());
    REQUIRE(ptr.use_count() == 1);
}

#endif
