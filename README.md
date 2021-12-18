[![master](https://github.com/Chylynsky/rc_ptr/actions/workflows/test-master.yml/badge.svg)](https://github.com/Chylynsky/rc_ptr/actions/workflows/test-master.yml)

# rc_ptr

Reference counted smart pointer for single threaded enviroments.

## Description

***rc_ptr*** class template manages shared ownership of an object of type **T** via the pointer. Multiple ***rc_ptr*** objects can manage the single object. The managed object is deleted when the last remaining ***rc_ptr*** object is either destroyed or reset. ***rc_ptr*** meets [**CopyConstructible**](https://en.cppreference.com/w/cpp/named_req/CopyConstructible), [**CopyAssignable**](https://en.cppreference.com/w/cpp/named_req/CopyAssignable), [**MoveConstructible**](https://en.cppreference.com/w/cpp/named_req/MoveConstructible) and [**MoveAssignable**](https://en.cppreference.com/w/cpp/named_req/MoveAssignable) requirements.

***weak_rc_ptr*** class template represents the non owning reference to the object managed by ***rc_ptr***. It can be used to break reference cycles. In order to obtain access to the managed object, the ***weak_rc_ptr*** instance must be converted to ***rc_ptr*** either by assignment or by the **lock** method. When the conversion fails, the ***bad_weak_rc_ptr*** exception is thrown.

***rc_ptr*** keeps track of the reference count by allocating the control block internally. By default, it is allocated and deallocated using the [**std::allocator<T>**](https://en.cppreference.com/w/cpp/memory/allocator). The control block is deallocated w1hen the count of both ***rc_ptr*** and ***weak_rc_ptr*** objects managing a single object reaches zero.

Custom deleters may be used to customize the objects destruction.

Custom allocator may be provided for internal use to allocate and later deallocate the control block.

Managing **this** pointer with ***rc_ptr*** directly is unsafe and will lead to undefined behaviour. This is what ***enable_rc_from_this*** is used for (see examples below).

Notes:
* It is, by design, *not* intended to be used in multithreaded enviroments due to lack of synchronization.
* Requires C++17.

## Examples

Include the header:

```cpp
#include "rc_ptr/rc_ptr.hpp"
```

Initialize with a pointer:

```cpp
using namespace memory;

rc_ptr<int> ptr{new int{24}};
assert(ptr); // Contextually convertible to bool
```

***rc_ptr*** objects can be dereferenced:

```cpp
using namespace memory;

rc_ptr<std::pair<int, float>> ptr{new std::pair<int, float>{ 0, 0.0f }};

std::pair<int, float> pair = *ptr;
ptr->first = 26;
ptr->second = 32.0f;
```

***rc_ptr*** objects are both move and copy constructible.
The internal reference count increases with each copy of the ***rc_ptr*** object. Current number of ***rc_ptr*** objects managing the pointer can be obtained by **use_count** method:

```cpp
using namespace memory;

rc_ptr<int> first{new int{24}};
rc_ptr<int> second = first;

assert(first.use_count() == 2);
assert(second.use_count() == 2);
```

Moving does not affect the reference count. 
Check for uniqueness can be made by using the unique method:

```cpp
using namespace memory;

rc_ptr<int> first{new int{24}};
rc_ptr<int> second = std::move(first);

assert(second.use_count() == 1)
```

Check if the managed object has expired by using the ***weak_rc_ptr***:

```cpp
using namespace memory;

weak_rc_ptr<int> weak;

{
    rc_ptr<int> ptr{new int{24}};
    weak = ptr;
    assert(!weak.expired());
} // ptr goes out of scope

assert(weak.expired());
```

***enable_rc_from_this*** is used to safely manage **this** pointer:

```cpp
class session : public enable_rc_from_this<session>
{
    // ...

    rc_ptr<session> rc_session()
    {
        return rc_from_this(); // Method derived from enable_rc_from_this
    }
};
```

### A word on namespaceing

By default, all the types described sit in the ***memory*** namespace. This can be changed by defining the RC_PTR_NAMESPACE macro with the namespace name you want BEFORE including the rc_ptr.hpp header:

```cpp
#define RC_PTR_NAMESPACE my_namespace
#include "rc_ptr/rc_ptr.hpp"
// ...
my_namespace::rc_ptr<float[]> = ...
```

## Documentation

In order to generate documentation use doxygen with the supplied configuration file.

```
doxygen .doxygen
```

## CI

CI pipeline consists of over a hundred of tests executed under [**Valgrind**](https://valgrind.org/).

## CMake

Example configuration using [**FetchContent**](https://cmake.org/cmake/help/latest/module/FetchContent.html#id1):

```
FetchContent_Declare(
    rc_ptr
    GIT_REPOSITORY    https://github.com/Chylynsky/rc_ptr.git
    GIT_TAG           master
)

FetchContent_MakeAvailable(
    rc_ptr
)
```

## Benchmark

Below are the benchmark results for copy operation of std::shared_ptr, rc_ptr and raw pointer. Benchmarked on Intel(R) Core(TM) i7-4710HQ CPU @ 2.50GHz.

Benchmark | Time | CPU | Iterations  
----------|------|-----|-----------
shared_ptr_copy | 11.3 ns | 11.3 ns | 60726146  
rc_ptr_copy | 3.59 ns | 3.59 ns | 193456420  
raw_ptr_copy | 0.307 ns | 0.307 ns | 1000000000  

## License

Copyright Borys Chyliński 2021.  
Distributed under the Boost Software License, Version 1.0.  
(See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)  

## Authors

Borys Chyliński
[@Chylynsky](https://github.com/Chylynsky)
