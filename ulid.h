#pragma once

#include "mtwister.h"
#include <stdint.h>

// ULID is a 16 byte Universally-unique Lexicographically-sortable IDentifier.
// https://github.com/ulid/spec

// Some constant for sizes:
enum {
  ULID_BYTES_ENTROPY = 10,
  ULID_BYTES_TIME = 6,
  ULID_BYTES_TOTAL = ULID_BYTES_ENTROPY + ULID_BYTES_TIME,
  ULID_BYTES_FORMATTED = 26,
};

// The kinds of entropy sources we support:
enum ULID_EntropyKind {
  ULID_ENTROPY_MERSENNE_TWISTER, // use Mersenne Twister
  ULID_ENTROPY_RAND,             // use rand() / srand()
};

// A factory which encapsulates all the state required to generate ULIDs.
// You can have multiple of these, each with their own configuration.
typedef struct ULID_Factory {
  uint64_t time_ms;                    // size:    8 bytes
  MTwister mt;                         // size: 2500 bytes
  uint32_t seed;                       // size:    4 bytes
  uint32_t calls;                      // size:    4 bytes
  uint8_t entropy[ULID_BYTES_ENTROPY]; // size:   10 bytes
  uint16_t flags;                      // size:    2 bytes
} ULID_Factory;                        // size: 2528 bytes

typedef struct ULID {
  uint8_t data[ULID_BYTES_TOTAL]; // size: 16 bytes
} ULID;

#ifndef __cplusplus
#if __STDC_VERSION__ >= 201112L
#include <assert.h>
// ensure there is no padding
static_assert(sizeof(ULID_Factory) == 2528, "ULID_Factory has size != 2528");
static_assert(sizeof(ULID) == 16, "ULID has size != 16");
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initialize a default ULID factory:
// * Use Mersenne Twister to generate entropy, seeded with gettimeofday().
// * Use gettimeofday() to generate timestamps in ms.
void ULID_Factory_Default(ULID_Factory *factory);

// Initialize a ULID factory with a specific entropy kind:
// * Mersenne Twister, seeded with gettimeofday() (default)
// * rand() / srand(), seeded with gettimeofday()
void ULID_Factory_SetEntropyKind(ULID_Factory *factory,
                                 const enum ULID_EntropyKind kind);

// Initialize a ULID factory where the entropy uses a specific seed.
void ULID_Factory_SetEntropySeed(ULID_Factory *factory, const uint32_t seed);

// Initialize a ULID factory where the starting entropy is given.
void ULID_Factory_SetEntropy(ULID_Factory *factory,
                             const uint8_t entropy[ULID_BYTES_ENTROPY]);

// Initialize a ULID factory where the starting time is given.
void ULID_Factory_SetTime(ULID_Factory *factory, const unsigned long time_ms);

// Create a ULID with the factory as configured.
void ULID_Create(ULID_Factory *factory, ULID *ulid);

// Get a ULID's time component.
unsigned ULID_GetTime(const ULID *ulid, unsigned long *time_ms);

// Get a ULID's entropy component.
unsigned ULID_GetEntropy(const ULID *ulid, uint8_t entropy[ULID_BYTES_ENTROPY]);

// Format a ULID's printable representation into a text buffer.
// Buffer must be at least ULID_BYTES_FORMATTED long.
// Buffer will NOT be zero-terminated.
// Return number of bytes generated.
unsigned ULID_Format(const ULID *ulid, char buf[ULID_BYTES_FORMATTED]);

// Parse a ULID from a string with its printable representation.
// String must be at least ULID_BYTES_FORMATTED long.
// String does NOT have to be zero-terminated.
// Return number of bytes generated.
unsigned ULID_Parse(ULID *ulid, const char str[ULID_BYTES_TOTAL]);

// Compare two ULIDs Lexicographically, returning:
//   l <  r => -1
//   l == r => 0
//   l >  r => +1
int ULID_Compare(const ULID *l, const ULID *r);

#ifdef __cplusplus
}
#endif
