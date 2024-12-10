# ULID implementation in C

Inspired on [this C++ implementation](https://github.com/suyash/ulid).

Order is guaranteed, even for ULIDs generated within the same ms
or with fixed time / entropy.

See tests and benchmarks (under directory `t`)
and command-line utility `culid.c`
for usage examples.

## Usage

Type `make help` for available build targets. Useful ones:
* `make all`: build library and dependent utilities.
* `make test`: run tests.
* `make bench`: run benchmarks.

When running the command-line utility `culid`,
you can get help with `culid -h`.
