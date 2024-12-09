#pragma once

#include "mtwister.h"
#include <stdint.h>

// ULID is a 16 byte Universally Unique Lexicographically Sortable Identifier.
// https://github.com/ulid/spec

enum {
  ULID_BYTES_ENTROPY = 10,
  ULID_BYTES_TIME = 6,
  ULID_BYTES_TOTAL = ULID_BYTES_ENTROPY + ULID_BYTES_TIME,
  ULID_BYTES_FORMATTED = 26,
};

enum ULID_EntropyKind {
  ULID_ENTROPY_RAND,
  ULID_ENTROPY_MERSENNE_TWISTER,
};

typedef struct ULID_Factory {
  MTwister mt;
  uint32_t seed;
  unsigned long time_ms;
  uint8_t entropy[ULID_BYTES_ENTROPY];
  uint8_t flags;
  uint32_t calls;
} ULID_Factory;

typedef struct ULID {
  uint8_t data[ULID_BYTES_TOTAL];
} ULID;

#ifdef __cplusplus
extern "C" {
#endif

void ULID_Factory_Init(ULID_Factory *factory);
void ULID_Factory_SetEntropyKind(ULID_Factory *factory,
                                 enum ULID_EntropyKind kind);
void ULID_Factory_SetEntropySeed(ULID_Factory *factory, uint32_t seed);
void ULID_Factory_SetEntropy(ULID_Factory *factory,
                             uint8_t entropy[ULID_BYTES_ENTROPY]);
void ULID_Factory_SetTime(ULID_Factory *factory, unsigned long time_ms);

void ULID_Create(ULID_Factory *factory, ULID *ulid);
unsigned ULID_Format(const ULID *ulid, char dst[ULID_BYTES_FORMATTED]);
unsigned ULID_Parse(ULID *ulid, char str[ULID_BYTES_TOTAL]);
int ULID_Compare(const ULID *l, const ULID *r);

#ifdef __cplusplus
}
#endif
