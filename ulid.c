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
  /*
   * We use strictly Crockford's Base32 alphabet when formatting.
   * Only digits and uppercase letters.
   * Skip letters I, L, O, U.
   */
  static const char Encode[33] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

#define EncN(p, m) (ulid->data[p] & (m))
#define EncL(p, m, s) ((ulid->data[p] & (m)) << (s))
#define EncR(p, m, s) ((ulid->data[p] & (m)) >> (s))

  // 10 byte timestamp
  buf[0x00] = Encode[EncR(0, 224, 5)];
  buf[0x01] = Encode[EncN(0, 31)];
  buf[0x02] = Encode[EncR(1, 248, 3)];
  buf[0x03] = Encode[EncL(1, 7, 2) | EncR(2, 192, 6)];
  buf[0x04] = Encode[EncR(2, 62, 1)];
  buf[0x05] = Encode[EncL(2, 1, 4) | EncR(3, 240, 4)];
  buf[0x06] = Encode[EncL(3, 15, 1) | EncR(4, 128, 7)];
  buf[0x07] = Encode[EncR(4, 124, 2)];
  buf[0x08] = Encode[EncL(4, 3, 3) | EncR(5, 224, 5)];
  buf[0x09] = Encode[EncN(5, 31)];

  // 16 bytes of entropy
  buf[0x0a] = Encode[EncR(6, 248, 3)];
  buf[0x0b] = Encode[EncL(6, 7, 2) | EncR(7, 192, 6)];
  buf[0x0c] = Encode[EncR(7, 62, 1)];
  buf[0x0d] = Encode[EncL(7, 1, 4) | EncR(8, 240, 4)];
  buf[0x0e] = Encode[EncL(8, 15, 1) | EncR(9, 128, 7)];
  buf[0x0f] = Encode[EncR(9, 124, 2)];
  buf[0x10] = Encode[EncL(9, 3, 3) | EncR(10, 224, 5)];
  buf[0x11] = Encode[EncN(10, 31)];
  buf[0x12] = Encode[EncR(11, 248, 3)];
  buf[0x13] = Encode[EncL(11, 7, 2) | EncR(12, 192, 6)];
  buf[0x14] = Encode[EncR(12, 62, 1)];
  buf[0x15] = Encode[EncL(12, 1, 4) | EncR(13, 240, 4)];
  buf[0x16] = Encode[EncL(13, 15, 1) | EncR(14, 128, 7)];
  buf[0x17] = Encode[EncR(14, 124, 2)];
  buf[0x18] = Encode[EncL(14, 3, 3) | EncR(15, 224, 5)];
  buf[0x19] = Encode[EncN(15, 31)];

  return ULID_BYTES_FORMATTED;
}

unsigned ULID_Parse(ULID *ulid, const char str[ULID_BYTES_TOTAL]) {
  /**
   * Decode stores decimal encodings for characters.
   * 0xFF indicates invalid character.
   *
   * We allow all uppercase and lowercase letters when parsing.
   * numerical digits: decimal 48 ~ 57, hex 30 ~ 39
   * uppercase letters: decimal 65 ~ 90, hex 41 ~ 5A
   * lowercase letters: decimal 97 ~ 122, hex 61 ~ 7A
   *
   * Crockford's Base32 skips letters I, L, O, U.
   * We assign them meaning:
   *
   *   I, i => 1
   *   L, l => 1
   *   O, o => 0
   *   U, u => 0
   */
  // clang-format off
  static const unsigned char Decode[256] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 //    0     1     2     3     4     5     6     7
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
 //    8     9
    0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 //       A 10  B 11  C 12  D 13  E 14  F 15  G 16
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
 // H 17  I  1  J 18  K 19  L  1  M 20  N 21  O  0
    0x11, 0x01, 0x12, 0x13, 0x01, 0x14, 0x15, 0x00,
 // P 22  Q 23  R 24  S 25  T 26  U  0  V 27  W 28
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x00, 0x1B, 0x1C,
 // X 29  Y 30  Z 31
    0x1D, 0x1E, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 //       a 10  b 11  c 12  d 13  e 14  f 15  g 16
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
 // h 17  i  1  j 18  k 19  l  1  m 20  n 21  o  0
    0x11, 0x01, 0x12, 0x13, 0x01, 0x14, 0x15, 0x00,
 // p 22  q 23  r 24  s 25  t 26  u  0  v 27  w 28
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x00, 0x1B, 0x1C,
 // x 29  y 30  z 31
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
  };
  // clang-format on

#define DecN(p) (Decode[(int)str[p]])
#define DecL(p, s) (Decode[(int)str[p]] << (s))
#define DecR(p, s) (Decode[(int)str[p]] >> (s))

  // timestamp
  ulid->data[0x0] = DecL(0, 5) | DecN(1);
  ulid->data[0x1] = DecL(2, 3) | DecR(3, 2);
  ulid->data[0x2] = DecL(3, 6) | DecL(4, 1) | DecR(5, 4);
  ulid->data[0x3] = DecL(5, 4) | DecR(6, 1);
  ulid->data[0x4] = DecL(6, 7) | DecL(7, 2) | DecR(8, 3);
  ulid->data[0x5] = DecL(8, 5) | DecN(9);

  // entropy
  ulid->data[0x6] = DecL(10, 3) | DecR(11, 2);
  ulid->data[0x7] = DecL(11, 6) | DecL(12, 1) | DecR(13, 4);
  ulid->data[0x8] = DecL(13, 4) | DecR(14, 1);
  ulid->data[0x9] = DecL(14, 7) | DecL(15, 2) | DecR(16, 3);
  ulid->data[0xa] = DecL(16, 5) | DecN(17);
  ulid->data[0xb] = DecL(18, 3) | DecR(19, 2);
  ulid->data[0xc] = DecL(19, 6) | DecL(20, 1) | DecR(21, 4);
  ulid->data[0xd] = DecL(21, 4) | DecR(22, 1);
  ulid->data[0xe] = DecL(22, 7) | DecL(23, 2) | DecR(24, 3);
  ulid->data[0xf] = DecL(24, 5) | DecN(25);

  return ULID_BYTES_TOTAL;
}

int ULID_Compare(const ULID *l, const ULID *r) {
#define CmpULID(ld, rd, p)                                                     \
  do {                                                                         \
    if (ld[p] != rd[p]) {                                                      \
      return (ld[p] < rd[p]) * -2 + 1;                                         \
    }                                                                          \
  } while (0)

  const uint8_t *ld = l->data;
  const uint8_t *rd = r->data;
#if 1
  for (unsigned p = 0; p < ULID_BYTES_TOTAL; ++p) {
    CmpULID(ld, rd, p);
  }
#else
  // hand-unrolled loop
  CmpULID(ld, rd, 0);
  CmpULID(ld, rd, 1);
  CmpULID(ld, rd, 2);
  CmpULID(ld, rd, 3);
  CmpULID(ld, rd, 4);
  CmpULID(ld, rd, 5);
  CmpULID(ld, rd, 6);
  CmpULID(ld, rd, 7);
  CmpULID(ld, rd, 8);
  CmpULID(ld, rd, 9);
  CmpULID(ld, rd, 10);
  CmpULID(ld, rd, 11);
  CmpULID(ld, rd, 12);
  CmpULID(ld, rd, 13);
  CmpULID(ld, rd, 14);
  CmpULID(ld, rd, 15);
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
