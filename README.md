[![build-master](https://github.com/Chylynsky/rc_ptr/actions/workflows/build-master.yml/badge.svg)](https://github.com/Chylynsky/rc_ptr/actions/workflows/build-master.yml) [![test-master](https://github.com/Chylynsky/rc_ptr/actions/workflows/test-master.yml/badge.svg)](https://github.com/Chylynsky/rc_ptr/actions/workflows/test-master.yml)

# rc_ptr

Implementation of reference counted smart pointer for single threaded enviroments.

## Description

This library provides a reference counter, non-synchronized smart pointer, witch statically dispatched deleters.

This library consists of below classes:
* rc_ptr
* weak_rc_ptr
* enable_rc_from_this
* bad_weak_rc_ptr

rc_ptr is a reference counted, non-synchronized smart pointer which releases ownership over the managed object when its internal reference count reaches zero. Arrays are supported. Custom deleters are statically dispatched, so their type must be known at compile time.

weak_rc_ptr may be used to break reference cycles. Current number of rc_ptr and weak_rc_ptr objects may be obtained at any time from weak_rc_ptr instance. weak_rc_ptr object may be converted to rc_ptr when the ownership is not yet released.

enable_rc_from_this is used to safely create rc_ptr objects from this pointer.

bad_weak_rc_ptr is as exception thrown when trying to obtain rc_ptr from expired weak_rc_ptr.

NOTE:
Currently the control block is allocated using new and delete.

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
