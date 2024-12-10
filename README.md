# ULID implementation in C

Inspired on [this C++ implementation](https://github.com/suyash/ulid).

Order is guaranteed, even for ULIDs generated within the same ms
or with fixed time / entropy.

See tests and benchmarks (under directory `t`) for usage examples.

## TODO
* Change `culid.c` to use command-line options to run in all possible modes.
