#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef struct ULID {
  uint8_t data[16];
} ULID;

static inline void EncodeTime(time_t timestamp, ULID *ulid) {
  ulid->data[0] = (uint8_t)(timestamp >> 40);
  ulid->data[1] = (uint8_t)(timestamp >> 32);
  ulid->data[2] = (uint8_t)(timestamp >> 24);
  ulid->data[3] = (uint8_t)(timestamp >> 16);
  ulid->data[4] = (uint8_t)(timestamp >> 8);
  ulid->data[5] = (uint8_t)(timestamp);
}

static inline unsigned EncodeEntropy(uint8_t rng[], ULID *ulid) {
  ulid->data[6] = rng[0];
  ulid->data[7] = rng[1];
  ulid->data[8] = rng[2];
  ulid->data[9] = rng[3];
  ulid->data[10] = rng[4];
  ulid->data[11] = rng[5];
  ulid->data[12] = rng[6];
  ulid->data[13] = rng[7];
  ulid->data[14] = rng[8];
  ulid->data[15] = rng[9];
  return 10;
}

#if 0
static std::uniform_int_distribution<rand_t> Distribution_0_255(0, 255);

/**
 * EncodeEntropyMt19937 will encode a ulid using std::mt19937
 *
 * It also creates a std::uniform_int_distribution to generate values in [0,
 * 255]
 * */
static inline void EncodeEntropyMt19937(std::mt19937 &generator, ULID &ulid) {
  ulid.data[6] = Distribution_0_255(generator);
  ulid.data[7] = Distribution_0_255(generator);
  ulid.data[8] = Distribution_0_255(generator);
  ulid.data[9] = Distribution_0_255(generator);
  ulid.data[10] = Distribution_0_255(generator);
  ulid.data[11] = Distribution_0_255(generator);
  ulid.data[12] = Distribution_0_255(generator);
  ulid.data[13] = Distribution_0_255(generator);
  ulid.data[14] = Distribution_0_255(generator);
  ulid.data[15] = Distribution_0_255(generator);
}
#endif

static inline const char *MarshalTo(const ULID *ulid, char dst[27]) {
  // 10 byte timestamp
  dst[0] = Encoding[(ulid->data[0] & 224) >> 5];
  dst[1] = Encoding[ulid->data[0] & 31];
  dst[2] = Encoding[(ulid->data[1] & 248) >> 3];
  dst[3] = Encoding[((ulid->data[1] & 7) << 2) | ((ulid->data[2] & 192) >> 6)];
  dst[4] = Encoding[(ulid->data[2] & 62) >> 1];
  dst[5] = Encoding[((ulid->data[2] & 1) << 4) | ((ulid->data[3] & 240) >> 4)];
  dst[6] = Encoding[((ulid->data[3] & 15) << 1) | ((ulid->data[4] & 128) >> 7)];
  dst[7] = Encoding[(ulid->data[4] & 124) >> 2];
  dst[8] = Encoding[((ulid->data[4] & 3) << 3) | ((ulid->data[5] & 224) >> 5)];
  dst[9] = Encoding[ulid->data[5] & 31];

  // 16 bytes of entropy
  dst[10] = Encoding[(ulid->data[6] & 248) >> 3];
  dst[11] = Encoding[((ulid->data[6] & 7) << 2) | ((ulid->data[7] & 192) >> 6)];
  dst[12] = Encoding[(ulid->data[7] & 62) >> 1];
  dst[13] = Encoding[((ulid->data[7] & 1) << 4) | ((ulid->data[8] & 240) >> 4)];
  dst[14] =
      Encoding[((ulid->data[8] & 15) << 1) | ((ulid->data[9] & 128) >> 7)];
  dst[15] = Encoding[(ulid->data[9] & 124) >> 2];
  dst[16] =
      Encoding[((ulid->data[9] & 3) << 3) | ((ulid->data[10] & 224) >> 5)];
  dst[17] = Encoding[ulid->data[10] & 31];
  dst[18] = Encoding[(ulid->data[11] & 248) >> 3];
  dst[19] =
      Encoding[((ulid->data[11] & 7) << 2) | ((ulid->data[12] & 192) >> 6)];
  dst[20] = Encoding[(ulid->data[12] & 62) >> 1];
  dst[21] =
      Encoding[((ulid->data[12] & 1) << 4) | ((ulid->data[13] & 240) >> 4)];
  dst[22] =
      Encoding[((ulid->data[13] & 15) << 1) | ((ulid->data[14] & 128) >> 7)];
  dst[23] = Encoding[(ulid->data[14] & 124) >> 2];
  dst[24] =
      Encoding[((ulid->data[14] & 3) << 3) | ((ulid->data[15] & 224) >> 5)];
  dst[25] = Encoding[ulid->data[15] & 31];
  dst[26] = '\0';
  return dst;
}

static inline void MarshalBinaryTo(const ULID *ulid, uint8_t dst[16]) {
  // timestamp
  dst[0] = ulid->data[0];
  dst[1] = ulid->data[1];
  dst[2] = ulid->data[2];
  dst[3] = ulid->data[3];
  dst[4] = ulid->data[4];
  dst[5] = ulid->data[5];

  // entropy
  dst[6] = ulid->data[6];
  dst[7] = ulid->data[7];
  dst[8] = ulid->data[8];
  dst[9] = ulid->data[9];
  dst[10] = ulid->data[10];
  dst[11] = ulid->data[11];
  dst[12] = ulid->data[12];
  dst[13] = ulid->data[13];
  dst[14] = ulid->data[14];
  dst[15] = ulid->data[15];
}

static inline void UnmarshalFrom(const char *str, ULID *ulid) {
  // timestamp
  ulid->data[0] = (dec[(int)str[0]] << 5) | dec[(int)str[1]];
  ulid->data[1] = (dec[(int)str[2]] << 3) | (dec[(int)str[3]] >> 2);
  ulid->data[2] = (dec[(int)str[3]] << 6) | (dec[(int)str[4]] << 1) |
                  (dec[(int)str[5]] >> 4);
  ulid->data[3] = (dec[(int)str[5]] << 4) | (dec[(int)str[6]] >> 1);
  ulid->data[4] = (dec[(int)str[6]] << 7) | (dec[(int)str[7]] << 2) |
                  (dec[(int)str[8]] >> 3);
  ulid->data[5] = (dec[(int)str[8]] << 5) | dec[(int)str[9]];

  // entropy
  ulid->data[6] = (dec[(int)str[10]] << 3) | (dec[(int)str[11]] >> 2);
  ulid->data[7] = (dec[(int)str[11]] << 6) | (dec[(int)str[12]] << 1) |
                  (dec[(int)str[13]] >> 4);
  ulid->data[8] = (dec[(int)str[13]] << 4) | (dec[(int)str[14]] >> 1);
  ulid->data[9] = (dec[(int)str[14]] << 7) | (dec[(int)str[15]] << 2) |
                  (dec[(int)str[16]] >> 3);
  ulid->data[10] = (dec[(int)str[16]] << 5) | dec[(int)str[17]];
  ulid->data[11] = (dec[(int)str[18]] << 3) | (dec[(int)str[19]] >> 2);
  ulid->data[12] = (dec[(int)str[19]] << 6) | (dec[(int)str[20]] << 1) |
                   (dec[(int)str[21]] >> 4);
  ulid->data[13] = (dec[(int)str[21]] << 4) | (dec[(int)str[22]] >> 1);
  ulid->data[14] = (dec[(int)str[22]] << 7) | (dec[(int)str[23]] << 2) |
                   (dec[(int)str[24]] >> 3);
  ulid->data[15] = (dec[(int)str[24]] << 5) | dec[(int)str[25]];
}

static inline void UnmarshalBinaryFrom(const uint8_t *b, ULID *ulid) {
  // timestamp
  ulid->data[0] = b[0];
  ulid->data[1] = b[1];
  ulid->data[2] = b[2];
  ulid->data[3] = b[3];
  ulid->data[4] = b[4];
  ulid->data[5] = b[5];

  // entropy
  ulid->data[6] = b[6];
  ulid->data[7] = b[7];
  ulid->data[8] = b[8];
  ulid->data[9] = b[9];
  ulid->data[10] = b[10];
  ulid->data[11] = b[11];
  ulid->data[12] = b[12];
  ulid->data[13] = b[13];
  ulid->data[14] = b[14];
  ulid->data[15] = b[15];
}

static inline int CompareULIDs(const ULID *ulid1, const ULID *ulid2) {
  // for (int i = 0 ; i < 16 ; i++) {
  // 	if (ulid1.data[i] != ulid2->data[i]) {
  // 		return (ulid1.data[i] < ulid2->data[i]) * -2 + 1;
  // 	}
  // }

  // unrolled loop

  if (ulid1->data[0] != ulid2->data[0]) {
    return (ulid1->data[0] < ulid2->data[0]) * -2 + 1;
  }

  if (ulid1->data[1] != ulid2->data[1]) {
    return (ulid1->data[1] < ulid2->data[1]) * -2 + 1;
  }

  if (ulid1->data[2] != ulid2->data[2]) {
    return (ulid1->data[2] < ulid2->data[2]) * -2 + 1;
  }

  if (ulid1->data[3] != ulid2->data[3]) {
    return (ulid1->data[3] < ulid2->data[3]) * -2 + 1;
  }

  if (ulid1->data[4] != ulid2->data[4]) {
    return (ulid1->data[4] < ulid2->data[4]) * -2 + 1;
  }

  if (ulid1->data[5] != ulid2->data[5]) {
    return (ulid1->data[5] < ulid2->data[5]) * -2 + 1;
  }

  if (ulid1->data[6] != ulid2->data[6]) {
    return (ulid1->data[6] < ulid2->data[6]) * -2 + 1;
  }

  if (ulid1->data[7] != ulid2->data[7]) {
    return (ulid1->data[7] < ulid2->data[7]) * -2 + 1;
  }

  if (ulid1->data[8] != ulid2->data[8]) {
    return (ulid1->data[8] < ulid2->data[8]) * -2 + 1;
  }

  if (ulid1->data[9] != ulid2->data[9]) {
    return (ulid1->data[9] < ulid2->data[9]) * -2 + 1;
  }

  if (ulid1->data[10] != ulid2->data[10]) {
    return (ulid1->data[10] < ulid2->data[10]) * -2 + 1;
  }

  if (ulid1->data[11] != ulid2->data[11]) {
    return (ulid1->data[11] < ulid2->data[11]) * -2 + 1;
  }

  if (ulid1->data[12] != ulid2->data[12]) {
    return (ulid1->data[12] < ulid2->data[12]) * -2 + 1;
  }

  if (ulid1->data[13] != ulid2->data[13]) {
    return (ulid1->data[13] < ulid2->data[13]) * -2 + 1;
  }

  if (ulid1->data[14] != ulid2->data[14]) {
    return (ulid1->data[14] < ulid2->data[14]) * -2 + 1;
  }

  if (ulid1->data[15] != ulid2->data[15]) {
    return (ulid1->data[15] < ulid2->data[15]) * -2 + 1;
  }

  return 0;
}

static inline time_t Time(const ULID *ulid) {
  time_t ans = 0;

  ans |= ulid->data[0];

  ans <<= 8;
  ans |= ulid->data[1];

  ans <<= 8;
  ans |= ulid->data[2];

  ans <<= 8;
  ans |= ulid->data[3];

  ans <<= 8;
  ans |= ulid->data[4];

  ans <<= 8;
  ans |= ulid->data[5];

  return ans;
}
