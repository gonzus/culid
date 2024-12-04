#pragma once

// ULID is a 16 byte Universally Unique Lexicographically Sortable Identifier.
// This is an implementation in C, header only.
// https://github.com/ulid/spec

#if defined(ULID_FORCE_UINT128) && defined(ULID_FORCE_STRUCT)
#error cannot define both ULID_FORCE_UINT128 and ULID_FORCE_STRUCT
#endif

#if defined(ULID_FORCE_UINT128)
#define ULID_USE_UINT128
#elif defined(ULID_FORCE_STRUCT)
#undef ULID_USE_UINT128
#elif defined(__SIZEOF_INT128__)
#define ULID_USE_UINT128
#endif

#include <stdint.h>
#include <sys/time.h>

/**
 * Crockford's Base32.
 */
static const char ULID_Encoding[33] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

/**
 * ULID_Decimal stores decimal encodings for characters.
 * 0xFF indicates invalid character.
 * 48-57 are digits.
 * 65-90 are capital letters.
 */
// clang-format off
static const uint8_t ULID_Decimal[256] = {
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

#if defined(ULID_USE_UINT128)
#include "ulid_uint128.h"
#else
#include "ulid_struct.h"
#endif

/**
 * ULID_EncodeTimeNow will encode a ULID using the time obtained using time(0).
 */
static inline void ULID_EncodeTimeNow(ULID *ulid) {
  // clang-format off
  ULID_EncodeTime(time(0), ulid);
  // clang-format on
}

/**
 * ULID_EncodeTimeSystemClockNow will encode a ULID using the time obtained
 * using gettimeofday() by taking the timestamp in milliseconds.
 */
static inline void ULID_EncodeTimeSystemClockNow(ULID *ulid) {
  struct timeval now;
  gettimeofday(&now, 0);
  unsigned long ms = now.tv_sec * 1000 + now.tv_usec / 1000;
  ULID_EncodeTime(ms, ulid);
}

/**
 * ULID_EncodeEntropyRand will encode a ulid using rand().
 *
 * rand() returns values in [0, RAND_MAX].
 */
static inline unsigned ULID_EncodeEntropyRand(ULID *ulid) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = rand() * 255ull / RAND_MAX;
  }
  return ULID_EncodeEntropy(rnd, ulid);
}

/**
 * ULID_Encode will create an encoded ULID with a timestamp and bytes taken from
 * an array (which must be pre-filled).
 */
static inline void ULID_Encode(time_t timestamp, uint8_t rnd[], ULID *ulid) {
  ULID_EncodeTime(timestamp, ulid);
  ULID_EncodeEntropy(rnd, ulid);
}

/**
 * ULID_EncodeNowRand = ULID_EncodeTimeNow + ULID_EncodeEntropyRand.
 */
static inline void ULID_EncodeNowRand(ULID *ulid) {
  ULID_EncodeTimeNow(ulid);
  ULID_EncodeEntropyRand(ulid);
}

/**
 * ULID_Create will create a ULID with a timestamp and a generator.
 */
static inline ULID ULID_Create(time_t timestamp, uint8_t rnd[]) {
  ULID ulid = {0};
  ULID_Encode(timestamp, rnd, &ulid);
  return ulid;
}

/**
 * ULID_CreateNowRand : ULID_EncodeNowRand = ULID_Create : ULID_Encode.
 */
static inline ULID ULID_CreateNowRand() {
  ULID ulid = {0};
  ULID_EncodeNowRand(&ulid);
  return ulid;
}

/**
 * ULID_EncodeTime will encode the first 6 bytes of a uint8_t array to the
 * passed timestamp.
 */
static inline void ULID_EncodeTime(time_t timestamp, ULID *ulid);

/**
 * ULID_EncodeEntropy will encode the last 10 bytes of the passed uint8_t array
 * with the values from a byte array.
 */
static inline unsigned ULID_EncodeEntropy(uint8_t rnd[], ULID *ulid);

/**
 * ULID_MarshalTo will marshal a ULID to the passed character array.
 *
 * Implementation taken directly from oklog/ulid
 * (https://sourcegraph.com/github.com/oklog/ulid@0774f81f6e44af5ce5e91c8d7d76cf710e889ebb/-/blob/ulid.go#L162-190)
 *
 * timestamp:<br>
 * dst[0]: first 3 bits of data[0]<br>
 * dst[1]: last 5 bits of data[0]<br>
 * dst[2]: first 5 bits of data[1]<br>
 * dst[3]: last 3 bits of data[1] + first 2 bits of data[2]<br>
 * dst[4]: bits 3-7 of data[2]<br>
 * dst[5]: last bit of data[2] + first 4 bits of data[3]<br>
 * dst[6]: last 4 bits of data[3] + first bit of data[4]<br>
 * dst[7]: bits 2-6 of data[4]<br>
 * dst[8]: last 2 bits of data[4] + first 3 bits of data[5]<br>
 * dst[9]: last 5 bits of data[5]<br>
 *
 * entropy:
 * follows similarly, except now all components are set to 5 bits.
 */
static inline const char *ULID_MarshalTo(const ULID *ulid, char dst[26]);

/**
 * ULID_MarshalBinaryTo will Marshal a ULID to the passed byte array.
 */
static inline void ULID_MarshalBinaryTo(const ULID *ulid, uint8_t dst[16]);

/**
 * ULID_UnmarshalFrom will unmarshal a ULID from the passed character array.
 */
static inline void ULID_UnmarshalFrom(const char *str, ULID *ulid);

/**
 * ULID_UnmarshalBinaryFrom will unmarshal a ULID from the passed byte array.
 */
static inline void ULID_UnmarshalBinaryFrom(const uint8_t *b, ULID *ulid);

/**
 * ULID_Compare will compare two ULIDs.
 * returns:
 *     -1 if ulid1 is Lexicographically before ulid2
 *      1 if ulid1 is Lexicographically after ulid2
 *      0 if ulid1 is same as ulid2
 */
static inline int ULID_Compare(const ULID *ulid1, const ULID *ulid2);

/**
 * ULID_Time will extract the timestamp used to generate a ULID.
 */
static inline time_t ULID_Time(const ULID *ulid);
