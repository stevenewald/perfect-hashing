# Simple Minimal Perfect Hashing Library

## TLDR

A lightweight [minimal perfect hashing](https://en.wikipedia.org/wiki/Perfect_hash_function) single-header library built to play around with C++ metaprogramming, compile-time optimizations, and neat hashing tricks.

## Examples

1. [Int → Int](https://godbolt.org/z/57jaTcqx4)
2. [String → Int](https://godbolt.org/z/9rjMezqev)
3. [Long String → Int](https://godbolt.org/z/46G1KWbdW)

## What is this?

A small side project to experiment with compile-time techniques in modern C++. The main parts are in [gloss.hpp](https://github.com/stevenewald/perfect-hashing/blob/main/include/gloss/gloss.hpp) and [gloss_test.cpp](https://github.com/stevenewald/perfect-hashing/blob/main/test/source/gloss_test.cpp).

## What is minimal perfect hashing?

A way to map exactly `n` unique keys to exactly `n` integers with zero collisions and no wasted space. Basically, you get a custom hash function perfectly tuned to your set of keys.

## Where is minimal perfect hashing used?

It's great when you have a known set of keys and need lightning-fast lookups, like mapping `"ETH"` to `Tickers::ETH` or an enum like `ErrorCode::NOT_FOUND` to `404`.

## How does compile-time metaprogramming fit in?

By doing all the heavy lifting at compile time (like figuring out the ideal hash function), the runtime lookups end up super simple and fast.


## What's next?

Adding CPU intrinsics and other optimizations for even faster lookups. Also, adding safer lookups with a `find` method that returns an optional instead of assuming keys always exist.

