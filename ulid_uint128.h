#pragma once

#include <stdlib.h>
#include <time.h>

typedef __uint128_t ULID;

static inline void ULID_EncodeTime(time_t timestamp, ULID *ulid) {
  ULID t = (uint8_t)(timestamp >> 40);

  t <<= 8;
  t |= (uint8_t)(timestamp >> 32);

  t <<= 8;
  t |= (uint8_t)(timestamp >> 24);

  t <<= 8;
  t |= (uint8_t)(timestamp >> 16);

  t <<= 8;
  t |= (uint8_t)(timestamp >> 8);

  t <<= 8;
  t |= (uint8_t)(timestamp);

  t <<= 80;

  ULID mask = 1;
  mask <<= 80;
  --mask;
  *ulid = t | (*ulid & mask);
}

static inline unsigned ULID_EncodeEntropy(uint8_t rnd[], ULID *ulid) {
  *ulid = (*ulid >> 80) << 80;

  ULID e = rnd[0];

  e <<= 8;
  e |= rnd[1];

  e <<= 8;
  e |= rnd[2];

  e <<= 8;
  e |= rnd[3];

  e <<= 8;
  e |= rnd[4];

  e <<= 8;
  e |= rnd[5];

  e <<= 8;
  e |= rnd[6];

  e <<= 8;
  e |= rnd[7];

  e <<= 8;
  e |= rnd[8];

  e <<= 8;
  e |= rnd[9];

  *ulid |= e;
  return 10;
}

static inline const char *ULID_MarshalTo(const ULID *ulid, char dst[27]) {
  // 10 byte timestamp
  dst[0] = ULID_Encoding[((uint8_t)(*ulid >> 120) & 224) >> 5];
  dst[1] = ULID_Encoding[(uint8_t)(*ulid >> 120) & 31];
  dst[2] = ULID_Encoding[((uint8_t)(*ulid >> 112) & 248) >> 3];
  dst[3] = ULID_Encoding[(((uint8_t)(*ulid >> 112) & 7) << 2) |
                         (((uint8_t)(*ulid >> 104) & 192) >> 6)];
  dst[4] = ULID_Encoding[((uint8_t)(*ulid >> 104) & 62) >> 1];
  dst[5] = ULID_Encoding[(((uint8_t)(*ulid >> 104) & 1) << 4) |
                         (((uint8_t)(*ulid >> 96) & 240) >> 4)];
  dst[6] = ULID_Encoding[(((uint8_t)(*ulid >> 96) & 15) << 1) |
                         (((uint8_t)(*ulid >> 88) & 128) >> 7)];
  dst[7] = ULID_Encoding[((uint8_t)(*ulid >> 88) & 124) >> 2];
  dst[8] = ULID_Encoding[(((uint8_t)(*ulid >> 88) & 3) << 3) |
                         (((uint8_t)(*ulid >> 80) & 224) >> 5)];
  dst[9] = ULID_Encoding[(uint8_t)(*ulid >> 80) & 31];

  // 16 bytes of entropy
  dst[10] = ULID_Encoding[((uint8_t)(*ulid >> 72) & 248) >> 3];
  dst[11] = ULID_Encoding[(((uint8_t)(*ulid >> 72) & 7) << 2) |
                          (((uint8_t)(*ulid >> 64) & 192) >> 6)];
  dst[12] = ULID_Encoding[((uint8_t)(*ulid >> 64) & 62) >> 1];
  dst[13] = ULID_Encoding[(((uint8_t)(*ulid >> 64) & 1) << 4) |
                          (((uint8_t)(*ulid >> 56) & 240) >> 4)];
  dst[14] = ULID_Encoding[(((uint8_t)(*ulid >> 56) & 15) << 1) |
                          (((uint8_t)(*ulid >> 48) & 128) >> 7)];
  dst[15] = ULID_Encoding[((uint8_t)(*ulid >> 48) & 124) >> 2];
  dst[16] = ULID_Encoding[(((uint8_t)(*ulid >> 48) & 3) << 3) |
                          (((uint8_t)(*ulid >> 40) & 224) >> 5)];
  dst[17] = ULID_Encoding[(uint8_t)(*ulid >> 40) & 31];
  dst[18] = ULID_Encoding[((uint8_t)(*ulid >> 32) & 248) >> 3];
  dst[19] = ULID_Encoding[(((uint8_t)(*ulid >> 32) & 7) << 2) |
                          (((uint8_t)(*ulid >> 24) & 192) >> 6)];
  dst[20] = ULID_Encoding[((uint8_t)(*ulid >> 24) & 62) >> 1];
  dst[21] = ULID_Encoding[(((uint8_t)(*ulid >> 24) & 1) << 4) |
                          (((uint8_t)(*ulid >> 16) & 240) >> 4)];
  dst[22] = ULID_Encoding[(((uint8_t)(*ulid >> 16) & 15) << 1) |
                          (((uint8_t)(*ulid >> 8) & 128) >> 7)];
  dst[23] = ULID_Encoding[((uint8_t)(*ulid >> 8) & 124) >> 2];
  dst[24] = ULID_Encoding[(((uint8_t)(*ulid >> 8) & 3) << 3) |
                          ((((uint8_t)(*ulid)) & 224) >> 5)];
  dst[25] = ULID_Encoding[((uint8_t)(*ulid)) & 31];

  dst[26] = '\0';
  return dst;
}

static inline void ULID_MarshalBinaryTo(const ULID *ulid, uint8_t dst[16]) {
  // timestamp
  dst[0] = (uint8_t)(*ulid >> 120);
  dst[1] = (uint8_t)(*ulid >> 112);
  dst[2] = (uint8_t)(*ulid >> 104);
  dst[3] = (uint8_t)(*ulid >> 96);
  dst[4] = (uint8_t)(*ulid >> 88);
  dst[5] = (uint8_t)(*ulid >> 80);

  // entropy
  dst[6] = (uint8_t)(*ulid >> 72);
  dst[7] = (uint8_t)(*ulid >> 64);
  dst[8] = (uint8_t)(*ulid >> 56);
  dst[9] = (uint8_t)(*ulid >> 48);
  dst[10] = (uint8_t)(*ulid >> 40);
  dst[11] = (uint8_t)(*ulid >> 32);
  dst[12] = (uint8_t)(*ulid >> 24);
  dst[13] = (uint8_t)(*ulid >> 16);
  dst[14] = (uint8_t)(*ulid >> 8);
  dst[15] = (uint8_t)(*ulid);
}

static inline void ULID_UnmarshalFrom(const char *str, ULID *ulid) {
  // timestamp
  *ulid = (ULID_Decimal[(int)(str[0])] << 5) | ULID_Decimal[(int)(str[1])];

  *ulid <<= 8;
  *ulid |=
      (ULID_Decimal[(int)(str[2])] << 3) | (ULID_Decimal[(int)(str[3])] >> 2);

  *ulid <<= 8;
  *ulid |= (ULID_Decimal[(int)(str[3])] << 6) |
           (ULID_Decimal[(int)(str[4])] << 1) |
           (ULID_Decimal[(int)(str[5])] >> 4);

  *ulid <<= 8;
  *ulid |=
      (ULID_Decimal[(int)(str[5])] << 4) | (ULID_Decimal[(int)(str[6])] >> 1);

  *ulid <<= 8;
  *ulid |= (ULID_Decimal[(int)(str[6])] << 7) |
           (ULID_Decimal[(int)(str[7])] << 2) |
           (ULID_Decimal[(int)(str[8])] >> 3);

  *ulid <<= 8;
  *ulid |= (ULID_Decimal[(int)(str[8])] << 5) | ULID_Decimal[(int)(str[9])];

  // entropy
  *ulid <<= 8;
  *ulid |=
      (ULID_Decimal[(int)(str[10])] << 3) | (ULID_Decimal[(int)(str[11])] >> 2);

  *ulid <<= 8;
  *ulid |= (ULID_Decimal[(int)(str[11])] << 6) |
           (ULID_Decimal[(int)(str[12])] << 1) |
           (ULID_Decimal[(int)(str[13])] >> 4);

  *ulid <<= 8;
  *ulid |=
      (ULID_Decimal[(int)(str[13])] << 4) | (ULID_Decimal[(int)(str[14])] >> 1);

  *ulid <<= 8;
  *ulid |= (ULID_Decimal[(int)(str[14])] << 7) |
           (ULID_Decimal[(int)(str[15])] << 2) |
           (ULID_Decimal[(int)(str[16])] >> 3);

  *ulid <<= 8;
  *ulid |= (ULID_Decimal[(int)(str[16])] << 5) | ULID_Decimal[(int)(str[17])];

  *ulid <<= 8;
  *ulid |=
      (ULID_Decimal[(int)(str[18])] << 3) | (ULID_Decimal[(int)(str[19])] >> 2);

  *ulid <<= 8;
  *ulid |= (ULID_Decimal[(int)(str[19])] << 6) |
           (ULID_Decimal[(int)(str[20])] << 1) |
           (ULID_Decimal[(int)(str[21])] >> 4);

  *ulid <<= 8;
  *ulid |=
      (ULID_Decimal[(int)(str[21])] << 4) | (ULID_Decimal[(int)(str[22])] >> 1);

  *ulid <<= 8;
  *ulid |= (ULID_Decimal[(int)(str[22])] << 7) |
           (ULID_Decimal[(int)(str[23])] << 2) |
           (ULID_Decimal[(int)(str[24])] >> 3);

  *ulid <<= 8;
  *ulid |= (ULID_Decimal[(int)(str[24])] << 5) | ULID_Decimal[(int)(str[25])];
}

static inline void ULID_UnmarshalBinaryFrom(const uint8_t *b, ULID *ulid) {
  // timestamp
  *ulid = b[0];

  *ulid <<= 8;
  *ulid |= b[1];

  *ulid <<= 8;
  *ulid |= b[2];

  *ulid <<= 8;
  *ulid |= b[3];

  *ulid <<= 8;
  *ulid |= b[4];

  *ulid <<= 8;
  *ulid |= b[5];

  // entropy
  *ulid <<= 8;
  *ulid |= b[6];

  *ulid <<= 8;
  *ulid |= b[7];

  *ulid <<= 8;
  *ulid |= b[8];

  *ulid <<= 8;
  *ulid |= b[9];

  *ulid <<= 8;
  *ulid |= b[10];

  *ulid <<= 8;
  *ulid |= b[11];

  *ulid <<= 8;
  *ulid |= b[12];

  *ulid <<= 8;
  *ulid |= b[13];

  *ulid <<= 8;
  *ulid |= b[14];

  *ulid <<= 8;
  *ulid |= b[15];
}

static inline int ULID_Compare(const ULID *ulid1, const ULID *ulid2) {
  return -2 * (*ulid1 < *ulid2) - 1 * (*ulid1 == *ulid2) + 1;
}

static inline time_t ULID_Time(const ULID *ulid) {
  time_t ans = 0;

  ans |= (uint8_t)(*ulid >> 120);

  ans <<= 8;
  ans |= (uint8_t)(*ulid >> 112);

  ans <<= 8;
  ans |= (uint8_t)(*ulid >> 104);

  ans <<= 8;
  ans |= (uint8_t)(*ulid >> 96);

  ans <<= 8;
  ans |= (uint8_t)(*ulid >> 88);

  ans <<= 8;
  ans |= (uint8_t)(*ulid >> 80);

  return ans;
}
