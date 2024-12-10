#include "ulid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

enum {
  ULID_FLAG_SEED = 1 << 0,
  ULID_FLAG_ENTROPY = 1 << 1,
  ULID_FLAG_TIME = 1 << 2,
  ULID_FLAG_USE_RAND = 1 << 3,
};

static inline void init_rand(uint32_t seed) {
  if (seed == 0) {
    struct timeval now;
    gettimeofday(&now, 0);
    seed = now.tv_usec;
  }
  srand(seed);
}

static inline void generate_time_ms(unsigned long *time_ms) {
  struct timeval now;
  gettimeofday(&now, 0);
  *time_ms = now.tv_sec * 1000 + now.tv_usec / 1000;
  // printf("time_ms %lu\n", time_ms);
}

static void inline generate_entropy(ULID_Factory *factory,
                                    uint8_t entropy[ULID_BYTES_ENTROPY]) {
  unsigned size = sizeof(uint32_t);
  for (unsigned pos = 0; pos < ULID_BYTES_ENTROPY;) {
    uint32_t random = 0;
    if (factory->flags & ULID_FLAG_USE_RAND) {
      random = rand();
    } else {
      random = mtwister_generate_u32(&factory->mt);
      // printf("RAND: %u\n", random);
    }
    unsigned copy = size;
    if (copy > ULID_BYTES_ENTROPY - pos) {
      copy = ULID_BYTES_ENTROPY - pos;
    }
    memcpy(entropy + pos, &random, copy);
    pos += copy;
  }
  // printf("ENTROPY: ");
  // for (unsigned p = 0; p < ULID_BYTES_ENTROPY; ++p) {
  //   printf("%02x", entropy[p]);
  // }
  // printf("\n");
}

void ULID_Factory_Default(ULID_Factory *factory) {
  memset(factory, 0, sizeof(ULID_Factory));
  init_rand(0);
  mtwister_build_from_random_seed(&factory->mt);
}

void ULID_Factory_SetEntropyKind(ULID_Factory *factory,
                                 const enum ULID_EntropyKind kind) {
  switch (kind) {
  case ULID_ENTROPY_RAND:
    factory->flags |= ULID_FLAG_USE_RAND;
    break;
  case ULID_ENTROPY_MERSENNE_TWISTER:
    factory->flags &= ~ULID_FLAG_USE_RAND;
    break;
  }
  init_rand(factory->seed);
  mtwister_build_from_seed(&factory->mt, factory->seed);
}

void ULID_Factory_SetEntropySeed(ULID_Factory *factory, const uint32_t seed) {
  factory->seed = seed;
  factory->flags |= ULID_FLAG_SEED;
  mtwister_build_from_seed(&factory->mt, factory->seed);
  init_rand(factory->seed);
}

void ULID_Factory_SetEntropy(ULID_Factory *factory,
                             const uint8_t entropy[ULID_BYTES_ENTROPY]) {
  memcpy(factory->entropy, entropy, ULID_BYTES_ENTROPY);
  factory->flags |= ULID_FLAG_ENTROPY;
}

void ULID_Factory_SetTime(ULID_Factory *factory, const unsigned long time_ms) {
  factory->time_ms = time_ms;
  factory->flags |= ULID_FLAG_TIME;
}

void ULID_Create(ULID_Factory *factory, ULID *ulid) {
  unsigned inc = 1;
  if (!(factory->flags & ULID_FLAG_TIME)) {
    unsigned long time_ms = 0;
    generate_time_ms(&time_ms);
    if (factory->time_ms != time_ms) {
      inc = 0;
    }
    factory->time_ms = time_ms;
  }
  ulid->data[0] = (unsigned char)(factory->time_ms >> 40);
  ulid->data[1] = (unsigned char)(factory->time_ms >> 32);
  ulid->data[2] = (unsigned char)(factory->time_ms >> 24);
  ulid->data[3] = (unsigned char)(factory->time_ms >> 16);
  ulid->data[4] = (unsigned char)(factory->time_ms >> 8);
  ulid->data[5] = (unsigned char)(factory->time_ms >> 0);

  if (!inc) {
    if (!(factory->flags & ULID_FLAG_ENTROPY)) {
      uint8_t entropy[ULID_BYTES_ENTROPY];
      generate_entropy(factory, entropy);
      memcpy(factory->entropy, entropy, ULID_BYTES_ENTROPY);
    }
  } else {
    if (!factory->calls) {
      uint8_t entropy[ULID_BYTES_ENTROPY];
      generate_entropy(factory, entropy);
      memcpy(factory->entropy, entropy, ULID_BYTES_ENTROPY);
    } else {
      for (unsigned p = 0; p < ULID_BYTES_ENTROPY; ++p) {
        unsigned n = ULID_BYTES_ENTROPY - p - 1;
        if (factory->entropy[n] < 0xff) {
          ++factory->entropy[n];
          break;
        } else {
          factory->entropy[n] = 0;
        }
      }
    }
  }
  memcpy(ulid->data + ULID_BYTES_TIME, factory->entropy, ULID_BYTES_ENTROPY);
  ++factory->calls;
}

unsigned ULID_Format(const ULID *ulid, char buf[ULID_BYTES_FORMATTED]) {
  // Crockford's Base32.
  static const char B32[33] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

  // 10 byte timestamp
  buf[0] = B32[(ulid->data[0] & 224) >> 5];
  buf[1] = B32[(ulid->data[0] & 31)];
  buf[2] = B32[(ulid->data[1] & 248) >> 3];
  buf[3] = B32[((ulid->data[1] & 7) << 2) | ((ulid->data[2] & 192) >> 6)];
  buf[4] = B32[(ulid->data[2] & 62) >> 1];
  buf[5] = B32[((ulid->data[2] & 1) << 4) | ((ulid->data[3] & 240) >> 4)];
  buf[6] = B32[((ulid->data[3] & 15) << 1) | ((ulid->data[4] & 128) >> 7)];
  buf[7] = B32[(ulid->data[4] & 124) >> 2];
  buf[8] = B32[((ulid->data[4] & 3) << 3) | ((ulid->data[5] & 224) >> 5)];
  buf[9] = B32[(ulid->data[5] & 31)];

  // 16 bytes of entropy
  buf[10] = B32[(ulid->data[6] & 248) >> 3];
  buf[11] = B32[((ulid->data[6] & 7) << 2) | ((ulid->data[7] & 192) >> 6)];
  buf[12] = B32[(ulid->data[7] & 62) >> 1];
  buf[13] = B32[((ulid->data[7] & 1) << 4) | ((ulid->data[8] & 240) >> 4)];
  buf[14] = B32[((ulid->data[8] & 15) << 1) | ((ulid->data[9] & 128) >> 7)];
  buf[15] = B32[(ulid->data[9] & 124) >> 2];
  buf[16] = B32[((ulid->data[9] & 3) << 3) | ((ulid->data[10] & 224) >> 5)];
  buf[17] = B32[(ulid->data[10] & 31)];
  buf[18] = B32[(ulid->data[11] & 248) >> 3];
  buf[19] = B32[((ulid->data[11] & 7) << 2) | ((ulid->data[12] & 192) >> 6)];
  buf[20] = B32[(ulid->data[12] & 62) >> 1];
  buf[21] = B32[((ulid->data[12] & 1) << 4) | ((ulid->data[13] & 240) >> 4)];
  buf[22] = B32[((ulid->data[13] & 15) << 1) | ((ulid->data[14] & 128) >> 7)];
  buf[23] = B32[(ulid->data[14] & 124) >> 2];
  buf[24] = B32[((ulid->data[14] & 3) << 3) | ((ulid->data[15] & 224) >> 5)];
  buf[25] = B32[(ulid->data[15] & 31)];
  return ULID_BYTES_FORMATTED;
}

unsigned ULID_Parse(ULID *ulid, const char str[ULID_BYTES_TOTAL]) {
  /**
   * Decimal stores decimal encodings for characters.
   * 0xFF indicates invalid character.
   * 48-57 are digits.
   * 65-90 are capital letters.
   */
  // clang-format off
  static const unsigned char Decimal[256] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0     1     2     3     4     5     6     7
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    // 8     9
    0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    //    10(A) 11(B) 12(C) 13(D) 14(E) 15(F) 16(G)
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    //17(H)     18(J) 19(K)       20(M) 21(N)
    0x11, 0xFF, 0x12, 0x13, 0xFF, 0x14, 0x15, 0xFF,
    //22(P)23(Q)24(R) 25(S) 26(T)       27(V) 28(W)
    0x16, 0x17, 0x18, 0x19, 0x1A, 0xFF, 0x1B, 0x1C,
    //29(X)30(Y)31(Z)
    0x1D, 0x1E, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  };
  // clang-format on

  // timestamp
  ulid->data[0] = (Decimal[(int)str[0]] << 5) | Decimal[(int)str[1]];
  ulid->data[1] = (Decimal[(int)str[2]] << 3) | (Decimal[(int)str[3]] >> 2);
  ulid->data[2] = (Decimal[(int)str[3]] << 6) | (Decimal[(int)str[4]] << 1) |
                  (Decimal[(int)str[5]] >> 4);
  ulid->data[3] = (Decimal[(int)str[5]] << 4) | (Decimal[(int)str[6]] >> 1);
  ulid->data[4] = (Decimal[(int)str[6]] << 7) | (Decimal[(int)str[7]] << 2) |
                  (Decimal[(int)str[8]] >> 3);
  ulid->data[5] = (Decimal[(int)str[8]] << 5) | Decimal[(int)str[9]];

  // entropy
  ulid->data[6] = (Decimal[(int)str[10]] << 3) | (Decimal[(int)str[11]] >> 2);
  ulid->data[7] = (Decimal[(int)str[11]] << 6) | (Decimal[(int)str[12]] << 1) |
                  (Decimal[(int)str[13]] >> 4);
  ulid->data[8] = (Decimal[(int)str[13]] << 4) | (Decimal[(int)str[14]] >> 1);
  ulid->data[9] = (Decimal[(int)str[14]] << 7) | (Decimal[(int)str[15]] << 2) |
                  (Decimal[(int)str[16]] >> 3);
  ulid->data[10] = (Decimal[(int)str[16]] << 5) | Decimal[(int)str[17]];
  ulid->data[11] = (Decimal[(int)str[18]] << 3) | (Decimal[(int)str[19]] >> 2);
  ulid->data[12] = (Decimal[(int)str[19]] << 6) | (Decimal[(int)str[20]] << 1) |
                   (Decimal[(int)str[21]] >> 4);
  ulid->data[13] = (Decimal[(int)str[21]] << 4) | (Decimal[(int)str[22]] >> 1);
  ulid->data[14] = (Decimal[(int)str[22]] << 7) | (Decimal[(int)str[23]] << 2) |
                   (Decimal[(int)str[24]] >> 3);
  ulid->data[15] = (Decimal[(int)str[24]] << 5) | Decimal[(int)str[25]];

  return ULID_BYTES_TOTAL;
}

int ULID_Compare(const ULID *l, const ULID *r) {
#define ULID_CMP(ld, rd, p)                                                    \
  do {                                                                         \
    if (ld[p] != rd[p]) {                                                      \
      return (ld[p] < rd[p]) * -2 + 1;                                         \
    }                                                                          \
  } while (0)

  const uint8_t *ld = l->data;
  const uint8_t *rd = r->data;
#if 1
  for (unsigned p = 0; p < ULID_BYTES_TOTAL; ++p) {
    ULID_CMP(ld, rd, p);
  }
#else
  // hand-unrolled loop
  ULID_CMP(ld, rd, 0);
  ULID_CMP(ld, rd, 1);
  ULID_CMP(ld, rd, 2);
  ULID_CMP(ld, rd, 3);
  ULID_CMP(ld, rd, 4);
  ULID_CMP(ld, rd, 5);
  ULID_CMP(ld, rd, 6);
  ULID_CMP(ld, rd, 7);
  ULID_CMP(ld, rd, 8);
  ULID_CMP(ld, rd, 9);
  ULID_CMP(ld, rd, 10);
  ULID_CMP(ld, rd, 11);
  ULID_CMP(ld, rd, 12);
  ULID_CMP(ld, rd, 13);
  ULID_CMP(ld, rd, 14);
  ULID_CMP(ld, rd, 15);
#endif
  return 0;
}

unsigned ULID_GetTime(const ULID *ulid, unsigned long *time_ms) {
  *time_ms = 0;
  for (unsigned p = 0; p < ULID_BYTES_TIME; ++p) {
    *time_ms <<= 8;
    *time_ms |= ulid->data[p];
  }
  return ULID_BYTES_TIME;
}

unsigned ULID_GetEntropy(const ULID *ulid,
                         uint8_t entropy[ULID_BYTES_ENTROPY]) {
  memcpy(entropy, ulid->data + ULID_BYTES_TIME, ULID_BYTES_ENTROPY);
  return ULID_BYTES_ENTROPY;
}
