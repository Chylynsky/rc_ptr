[![build-master](https://github.com/Chylynsky/rc_ptr/actions/workflows/build-master.yml/badge.svg)](https://github.com/Chylynsky/rc_ptr/actions/workflows/build-master.yml) [![test-master](https://github.com/Chylynsky/rc_ptr/actions/workflows/test-master.yml/badge.svg)](https://github.com/Chylynsky/rc_ptr/actions/workflows/test-master.yml) [![build-dev](https://github.com/Chylynsky/rc_ptr/actions/workflows/build-dev.yml/badge.svg)](https://github.com/Chylynsky/rc_ptr/actions/workflows/build-dev.yml) [![test-dev](https://github.com/Chylynsky/rc_ptr/actions/workflows/test-dev.yml/badge.svg)](https://github.com/Chylynsky/rc_ptr/actions/workflows/test-dev.yml)


# rc_ptr

Implementation of reference counted smart pointer for single threaded enviroments.

## Description

Work in progress.

## Benchmark

Below are the benchmark results for copy operation of std::shared_ptr, rc_ptr and raw pointer. Lack of synchronization allows for over three times faster copy operations of rc_ptr when compared to std::shared_ptr.

----------------------------------------------------------
Benchmark                Time             CPU   Iterations
----------------------------------------------------------
shared_ptr_copy       11.3 ns         11.3 ns     60726146
rc_ptr_copy           3.59 ns         3.59 ns    193456420
raw_ptr_copy         0.307 ns        0.307 ns   1000000000

## License

Copyright Borys Chyliński 2021.  
Distributed under the Boost Software License, Version 1.0.  
(See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)  

## Authors

Borys Chyliński
[@Chylynsky](https://github.com/Chylynsky)
