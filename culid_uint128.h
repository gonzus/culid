#pragma once

// #include <chrono>
// #include <cstdint>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
// #include <functional>
// #include <random>
// #include <vector>

/**
 * ULID is a 16 byte Universally Unique Lexicographically Sortable Identifier
 * */
typedef __uint128_t ULID;

/**
 * EncodeTime will encode the first 6 bytes of a uint8_t array to the passed
 * timestamp
 * */
static inline void EncodeTime(time_t timestamp, ULID *ulid) {
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

/**
 * EncodeTimeNow will encode a ULID using the time obtained using
 * std::time(nullptr)
 * */
static inline void EncodeTimeNow(ULID *ulid) {
  // gonzo
  EncodeTime(time(0), ulid);
}

/**
 * EncodeTimeSystemClockNow will encode a ULID using the time obtained using
 * gettimeofday() by taking the timestamp in milliseconds.
 * */
static inline void EncodeTimeSystemClockNow(ULID *ulid) {
  struct timeval now;
  gettimeofday(&now, 0);
  unsigned long ms = now.tv_sec * 1000 + now.tv_usec / 1000;
  EncodeTime(ms, ulid);
}

/**
 * EncodeEntropy will encode the last 10 bytes of the passed uint8_t array with
 * the values generated using the passed random number generator.
 * */
static inline unsigned EncodeEntropy(uint8_t rng[], ULID *ulid) {
  *ulid = (*ulid >> 80) << 80;

  ULID e = rng[0];

  e <<= 8;
  e |= rng[1];

  e <<= 8;
  e |= rng[2];

  e <<= 8;
  e |= rng[3];

  e <<= 8;
  e |= rng[4];

  e <<= 8;
  e |= rng[5];

  e <<= 8;
  e |= rng[6];

  e <<= 8;
  e |= rng[7];

  e <<= 8;
  e |= rng[8];

  e <<= 8;
  e |= rng[9];

  *ulid |= e;
  return 10;
}

/**
 * EncodeEntropyRand will encode a ulid using rand().
 *
 * rand() returns values in [0, RAND_MAX]
 * */
static inline unsigned EncodeEntropyRand(ULID *ulid) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = rand() * 255ull / RAND_MAX;
  }
  return EncodeEntropy(rng, ulid);
}

#if 0
static std::uniform_int_distribution<rand_t) Distribution_0_255(0, 255);

/**
 * EncodeEntropyMt19937 will encode a ulid using std::mt19937
 *
 * It also creates a std::uniform_int_distribution to generate values in [0,
 * 255]
 * */
static inline void EncodeEntropyMt19937(std::mt19937 &generator, ULID *ulid) {
  ulid = (ulid >> 80) << 80;

  ULID e = Distribution_0_255(generator);

  e <<= 8;
  e |= Distribution_0_255(generator);

  e <<= 8;
  e |= Distribution_0_255(generator);

  e <<= 8;
  e |= Distribution_0_255(generator);

  e <<= 8;
  e |= Distribution_0_255(generator);

  e <<= 8;
  e |= Distribution_0_255(generator);

  e <<= 8;
  e |= Distribution_0_255(generator);

  e <<= 8;
  e |= Distribution_0_255(generator);

  e <<= 8;
  e |= Distribution_0_255(generator);

  e <<= 8;
  e |= Distribution_0_255(generator);

  ulid |= e;
}
#endif

/**
 * Encode will create an encoded ULID with a timestamp and a generator.
 * */
static inline void Encode(time_t timestamp, uint8_t rng[], ULID *ulid) {
  EncodeTime(timestamp, ulid);
  EncodeEntropy(rng, ulid);
}

/**
 * EncodeNowRand = EncodeTimeNow + EncodeEntropyRand.
 * */
static inline void EncodeNowRand(ULID *ulid) {
  EncodeTimeNow(ulid);
  EncodeEntropyRand(ulid);
}

/**
 * Create will create a ULID with a timestamp and a generator.
 * */
static inline ULID Create(time_t timestamp, uint8_t rng[]) {
  ULID ulid = 0;
  Encode(timestamp, rng, &ulid);
  return ulid;
}

/**
 * CreateNowRand:EncodeNowRand = Create:Encode.
 * */
static inline ULID CreateNowRand() {
  ULID ulid = 0;
  EncodeNowRand(&ulid);
  return ulid;
}

/**
 * Crockford's Base32
 * */
static const char Encoding[33] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

/**
 * MarshalTo will marshal a ULID to the passed character array.
 *
 * Implementation taken directly from oklog/ulid
 * (https://sourcegraph.com/github.com/oklog/ulid@0774f81f6e44af5ce5e91c8d7d76cf710e889ebb/-/blob/ulid.go#L162-190)
 *
 * timestamp:
 * dst[0]: first 3 bits of data[0]
 * dst[1]: last 5 bits of data[0]
 * dst[2]: first 5 bits of data[1]
 * dst[3]: last 3 bits of data[1] + first 2 bits of data[2]
 * dst[4]: bits 3-7 of data[2]
 * dst[5]: last bit of data[2] + first 4 bits of data[3]
 * dst[6]: last 4 bits of data[3] + first bit of data[4]
 * dst[7]: bits 2-6 of data[4]
 * dst[8]: last 2 bits of data[4] + first 3 bits of data[5]
 * dst[9]: last 5 bits of data[5]
 *
 * entropy:
 * follows similarly, except now all components are set to 5 bits.
 * */
static inline const char *MarshalTo(const ULID *ulid, char dst[27]) {
  // 10 byte timestamp
  dst[0] = Encoding[((uint8_t)(*ulid >> 120) & 224) >> 5];
  dst[1] = Encoding[(uint8_t)(*ulid >> 120) & 31];
  dst[2] = Encoding[((uint8_t)(*ulid >> 112) & 248) >> 3];
  dst[3] = Encoding[(((uint8_t)(*ulid >> 112) & 7) << 2) |
                    (((uint8_t)(*ulid >> 104) & 192) >> 6)];
  dst[4] = Encoding[((uint8_t)(*ulid >> 104) & 62) >> 1];
  dst[5] = Encoding[(((uint8_t)(*ulid >> 104) & 1) << 4) |
                    (((uint8_t)(*ulid >> 96) & 240) >> 4)];
  dst[6] = Encoding[(((uint8_t)(*ulid >> 96) & 15) << 1) |
                    (((uint8_t)(*ulid >> 88) & 128) >> 7)];
  dst[7] = Encoding[((uint8_t)(*ulid >> 88) & 124) >> 2];
  dst[8] = Encoding[(((uint8_t)(*ulid >> 88) & 3) << 3) |
                    (((uint8_t)(*ulid >> 80) & 224) >> 5)];
  dst[9] = Encoding[(uint8_t)(*ulid >> 80) & 31];

  // 16 bytes of entropy
  dst[10] = Encoding[((uint8_t)(*ulid >> 72) & 248) >> 3];
  dst[11] = Encoding[(((uint8_t)(*ulid >> 72) & 7) << 2) |
                     (((uint8_t)(*ulid >> 64) & 192) >> 6)];
  dst[12] = Encoding[((uint8_t)(*ulid >> 64) & 62) >> 1];
  dst[13] = Encoding[(((uint8_t)(*ulid >> 64) & 1) << 4) |
                     (((uint8_t)(*ulid >> 56) & 240) >> 4)];
  dst[14] = Encoding[(((uint8_t)(*ulid >> 56) & 15) << 1) |
                     (((uint8_t)(*ulid >> 48) & 128) >> 7)];
  dst[15] = Encoding[((uint8_t)(*ulid >> 48) & 124) >> 2];
  dst[16] = Encoding[(((uint8_t)(*ulid >> 48) & 3) << 3) |
                     (((uint8_t)(*ulid >> 40) & 224) >> 5)];
  dst[17] = Encoding[(uint8_t)(*ulid >> 40) & 31];
  dst[18] = Encoding[((uint8_t)(*ulid >> 32) & 248) >> 3];
  dst[19] = Encoding[(((uint8_t)(*ulid >> 32) & 7) << 2) |
                     (((uint8_t)(*ulid >> 24) & 192) >> 6)];
  dst[20] = Encoding[((uint8_t)(*ulid >> 24) & 62) >> 1];
  dst[21] = Encoding[(((uint8_t)(*ulid >> 24) & 1) << 4) |
                     (((uint8_t)(*ulid >> 16) & 240) >> 4)];
  dst[22] = Encoding[(((uint8_t)(*ulid >> 16) & 15) << 1) |
                     (((uint8_t)(*ulid >> 8) & 128) >> 7)];
  dst[23] = Encoding[((uint8_t)(*ulid >> 8) & 124) >> 2];
  dst[24] = Encoding[(((uint8_t)(*ulid >> 8) & 3) << 3) |
                     ((((uint8_t)(*ulid)) & 224) >> 5)];
  dst[25] = Encoding[((uint8_t)(*ulid)) & 31];

  dst[26] = '\0';
  return dst;
}

/**
 * MarshalBinaryTo will Marshal a ULID to the passed byte array
 * */
static inline void MarshalBinaryTo(const ULID *ulid, uint8_t dst[16]) {
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

/**
 * dec storesdecimal encodings for characters.
 * 0xFF indicates invalid character.
 * 48-57 are digits.
 * 65-90 are capital alphabets.
 * */
static const uint8_t dec[256] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    /* 0     1     2     3     4     5     6     7  */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    /* 8     9                                      */
    0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /*    10(A) 11(B) 12(C) 13(D) 14(E) 15(F) 16(G) */
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    /*17(H)     18(J) 19(K)       20(M) 21(N)       */
    0x11, 0xFF, 0x12, 0x13, 0xFF, 0x14, 0x15, 0xFF,
    /*22(P)23(Q)24(R) 25(S) 26(T)       27(V) 28(W) */
    0x16, 0x17, 0x18, 0x19, 0x1A, 0xFF, 0x1B, 0x1C,
    /*29(X)30(Y)31(Z)                               */
    0x1D, 0x1E, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/**
 * UnmarshalFrom will unmarshal a ULID from the passed character array.
 * */
static inline void UnmarshalFrom(const char *str, ULID *ulid) {
  // timestamp
  *ulid = (dec[(int)(str[0])] << 5) | dec[(int)(str[1])];

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[2])] << 3) | (dec[(int)(str[3])] >> 2);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[3])] << 6) | (dec[(int)(str[4])] << 1) |
           (dec[(int)(str[5])] >> 4);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[5])] << 4) | (dec[(int)(str[6])] >> 1);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[6])] << 7) | (dec[(int)(str[7])] << 2) |
           (dec[(int)(str[8])] >> 3);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[8])] << 5) | dec[(int)(str[9])];

  // entropy
  *ulid <<= 8;
  *ulid |= (dec[(int)(str[10])] << 3) | (dec[(int)(str[11])] >> 2);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[11])] << 6) | (dec[(int)(str[12])] << 1) |
           (dec[(int)(str[13])] >> 4);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[13])] << 4) | (dec[(int)(str[14])] >> 1);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[14])] << 7) | (dec[(int)(str[15])] << 2) |
           (dec[(int)(str[16])] >> 3);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[16])] << 5) | dec[(int)(str[17])];

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[18])] << 3) | (dec[(int)(str[19])] >> 2);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[19])] << 6) | (dec[(int)(str[20])] << 1) |
           (dec[(int)(str[21])] >> 4);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[21])] << 4) | (dec[(int)(str[22])] >> 1);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[22])] << 7) | (dec[(int)(str[23])] << 2) |
           (dec[(int)(str[24])] >> 3);

  *ulid <<= 8;
  *ulid |= (dec[(int)(str[24])] << 5) | dec[(int)(str[25])];
}

/**
 * UnmarshalBinaryFrom will unmarshal a ULID from the passed byte array.
 * */
static inline void UnmarshalBinaryFrom(const uint8_t *b, ULID *ulid) {
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

/**
 * CompareULIDs will compare two ULIDs.
 * returns:
 *     -1 if ulid1 is Lexicographically before ulid2
 *      1 if ulid1 is Lexicographically after ulid2
 *      0 if ulid1 is same as ulid2
 * */
static inline int CompareULIDs(const ULID *ulid1, const ULID *ulid2) {
  return -2 * (*ulid1 < *ulid2) - 1 * (*ulid1 == *ulid2) + 1;
}

/**
 * Time will extract the timestamp used to generate a ULID
 * */
static inline time_t Time(const ULID *ulid) {
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
