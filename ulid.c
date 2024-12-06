#include "ulid.h"
// #include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

enum {
  ULID_FLAG_SEED = 1 << 0,
  ULID_FLAG_ENTROPY = 1 << 1,
  ULID_FLAG_TIME = 1 << 2,
};

void ULID_Factory_Init(ULID_Factory *factory) {
  memset(factory, 0, sizeof(ULID_Factory));
  mtwister_build_from_random_seed(&factory->mt);
}

void ULID_Factory_SetEntropySeed(ULID_Factory *factory, uint32_t seed) {
  factory->seed = seed;
  factory->flags |= ULID_FLAG_SEED;
  mtwister_build_from_seed(&factory->mt, seed);
}

void ULID_Factory_SetEntropy(ULID_Factory *factory,
                             uint8_t entropy[ULID_BYTES_ENTROPY]) {
  memcpy(factory->entropy, entropy, ULID_BYTES_ENTROPY);
  factory->flags |= ULID_FLAG_ENTROPY;
}

void ULID_Factory_SetTime(ULID_Factory *factory, unsigned long time_ms) {
  factory->time_ms = time_ms;
  factory->flags |= ULID_FLAG_TIME;
}

void ULID_Create(ULID_Factory *factory, ULID *ulid) {
  unsigned long time_ms = 0;
  if (factory->flags & ULID_FLAG_TIME) {
    time_ms = factory->time_ms;
  } else {
    struct timeval now;
    gettimeofday(&now, 0);
    time_ms = now.tv_sec * 1000 + now.tv_usec / 1000;
    // printf("time_ms %lu\n", time_ms);
  }
  ulid->data[0] = (unsigned char)(time_ms >> 40);
  ulid->data[1] = (unsigned char)(time_ms >> 32);
  ulid->data[2] = (unsigned char)(time_ms >> 24);
  ulid->data[3] = (unsigned char)(time_ms >> 16);
  ulid->data[4] = (unsigned char)(time_ms >> 8);
  ulid->data[5] = (unsigned char)(time_ms >> 0);

  if (factory->flags & ULID_FLAG_ENTROPY) {
    memcpy(ulid->data + ULID_BYTES_TIME, factory->entropy, ULID_BYTES_ENTROPY);
  } else {
    unsigned size = sizeof(uint32_t);
    for (unsigned pos = 0; pos < ULID_BYTES_ENTROPY;) {
      uint32_t random = mtwister_generate_u32(&factory->mt);
      unsigned copy = size;
      if (copy > ULID_BYTES_ENTROPY - pos) {
        copy = ULID_BYTES_ENTROPY - pos;
      }
      memcpy(ulid->data + ULID_BYTES_TIME + pos, &random, copy);
      pos += copy;
    }
  }
}

unsigned ULID_Format(const ULID *ulid, char dst[ULID_BYTES_FORMATTED]) {
  // Crockford's Base32.
  static const char B32[33] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

  // 10 byte timestamp
  dst[0] = B32[(ulid->data[0] & 224) >> 5];
  dst[1] = B32[(ulid->data[0] & 31)];
  dst[2] = B32[(ulid->data[1] & 248) >> 3];
  dst[3] = B32[((ulid->data[1] & 7) << 2) | ((ulid->data[2] & 192) >> 6)];
  dst[4] = B32[(ulid->data[2] & 62) >> 1];
  dst[5] = B32[((ulid->data[2] & 1) << 4) | ((ulid->data[3] & 240) >> 4)];
  dst[6] = B32[((ulid->data[3] & 15) << 1) | ((ulid->data[4] & 128) >> 7)];
  dst[7] = B32[(ulid->data[4] & 124) >> 2];
  dst[8] = B32[((ulid->data[4] & 3) << 3) | ((ulid->data[5] & 224) >> 5)];
  dst[9] = B32[(ulid->data[5] & 31)];

  // 16 bytes of entropy
  dst[10] = B32[(ulid->data[6] & 248) >> 3];
  dst[11] = B32[((ulid->data[6] & 7) << 2) | ((ulid->data[7] & 192) >> 6)];
  dst[12] = B32[(ulid->data[7] & 62) >> 1];
  dst[13] = B32[((ulid->data[7] & 1) << 4) | ((ulid->data[8] & 240) >> 4)];
  dst[14] = B32[((ulid->data[8] & 15) << 1) | ((ulid->data[9] & 128) >> 7)];
  dst[15] = B32[(ulid->data[9] & 124) >> 2];
  dst[16] = B32[((ulid->data[9] & 3) << 3) | ((ulid->data[10] & 224) >> 5)];
  dst[17] = B32[(ulid->data[10] & 31)];
  dst[18] = B32[(ulid->data[11] & 248) >> 3];
  dst[19] = B32[((ulid->data[11] & 7) << 2) | ((ulid->data[12] & 192) >> 6)];
  dst[20] = B32[(ulid->data[12] & 62) >> 1];
  dst[21] = B32[((ulid->data[12] & 1) << 4) | ((ulid->data[13] & 240) >> 4)];
  dst[22] = B32[((ulid->data[13] & 15) << 1) | ((ulid->data[14] & 128) >> 7)];
  dst[23] = B32[(ulid->data[14] & 124) >> 2];
  dst[24] = B32[((ulid->data[14] & 3) << 3) | ((ulid->data[15] & 224) >> 5)];
  dst[25] = B32[(ulid->data[15] & 31)];
  return ULID_BYTES_FORMATTED;
}

unsigned ULID_Parse(ULID *ulid, char str[ULID_BYTES_TOTAL]) {
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
  for (unsigned p = 0; p < 16; ++p) {
    if (l->data[p] != r->data[p]) {
      return (l->data[p] < r->data[p]) * -2 + 1;
    }
  }
  return 0;
}

#if 0
unsigned ULID_GetTime(const ULID *ulid, unsigned long *time) {
  *time = 0;
  for (unsigned p = 0; p < 6; ++p) {
    *time <<= 8;
    *time |= ulid->data[p];
  }
  return 6;
}

unsigned ULID_GetEntropy(const ULID *ulid, uint8_t entropy[10]) {
  memcpy(entropy, ulid->data + 6, 10);
  return 10;
}

static unsigned EncodeTime(ULID *ulid, unsigned long time_ms) {
  ulid->data[0] = (unsigned char)(time_ms >> 40);
  ulid->data[1] = (unsigned char)(time_ms >> 32);
  ulid->data[2] = (unsigned char)(time_ms >> 24);
  ulid->data[3] = (unsigned char)(time_ms >> 16);
  ulid->data[4] = (unsigned char)(time_ms >> 8);
  ulid->data[5] = (unsigned char)(time_ms >> 0);
  return 6;
}

static unsigned EncodeEntropy(ULID *ulid) {
  if (!mt_built) {
    mtwister_build_from_random_seed(&mt);
    mt_built = 1;
  }
  uint32_t r0 = mtwister_generate_u32(&mt);
  memcpy(ulid->data + 6, &r0, 4);
  uint32_t r1 = mtwister_generate_u32(&mt);
  memcpy(ulid->data + 10, &r1, 4);
  uint32_t r2 = mtwister_generate_u32(&mt);
  memcpy(ulid->data + 14, &r2, 2);
  return 16;
}
#endif
