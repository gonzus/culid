# ULID implementation in C

Inspired on [this C++ implementation](https://github.com/suyash/ulid).

Order is guaranteed, even for ULIDs generated within the same millisecond
or with fixed time / entropy.

See tests and benchmarks (under directory `t`)
and command-line utility `culid.c`
for usage examples.

## Usage

Type `make help` for available build targets.  Some useful targets are:
* `make all`: build library and utilities.
* `make test`: run tests.
* `make bench`: run benchmarks.

When running the command-line utility `culid`,
you can get help with `culid -h`.

## API

For full details of the API, please read the comments in `ulid.h`.

The easiest, default way to create ULIDs with the library is to create a default factory and then generate ULIDs:
```C
// This factory has all the context needed to create ULIDs.
// You can have several of these, each with its own configuration.
ULID_Factory uf;
ULID_Factory_Default(&uf);

// Each factory can create ULIDs according to its configuration.
ULID ulid;
ULID_Create(uf, &ulid);

// You can also create ULIDs in a tight loop; they will
// still be guaranteed to be unique and correctly sorted.
for (unsigned n = 0; n < 1000000; ++n) {
  ULID ulid;
  ULID_Create(uf, &ulid);

  char txt[ULID_BYTES_FORMATTED];
  ULID_Format(&ulid, txt); // notice, NOT null terminated
  printf("ULID #%6d: [%.*s]\n", n, ULID_BYTES_FORMATTED, txt);
}
```

Additionally, you can configure a ULID factory with several options:
* Setting the source for entropy.  The default is using an
  internal implementation of
  [Mersenne Twister](https://en.wikipedia.org/wiki/Mersenne_Twister),
  you can also choose to use
  [rand() / srand()](https://linux.die.net/man/3/srand).
* Setting a seed for the entropy generation.  The default is
  seeding the pseudo-random number generator by calling
  [gettimeofday()](https://linux.die.net/man/2/gettimeofday)
  and using `tv_usec` as the seed.
* Setting a fixed value for the entropy -- an array of bytes.
  Useful to test specific values while generating ULIDs.
* Setting a fixed value for the time -- a number of milliseconds.
  Useful to test specific values while generating ULIDs.

Once you have created a couple of ULIDs, you can:
* Get their time component.
* Get their entropy component.
* Format them as a printable string.
* Compare them ULIDs with the typical `-1`, `0`, `+1` semantics.

You can also create a ULID by parsing a string formatted as a printable string.
