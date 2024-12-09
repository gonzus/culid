#include <gtest/gtest.h>

#include <cstring>
#include <ctime>
#include <ulid.h>

enum {
  NUMBER_OF_ULIDS = 500,
  MS_BETWEEN_ULIDS = 1,
};

TEST(culid, basic_creation_and_formatting) {
  ULID_Factory uf;
  ULID_Factory_Init(&uf);

  ULID ulid;
  ULID_Create(&uf, &ulid);

  char txt[26];
  ULID_Format(&ulid, txt);

  for (unsigned p = 0; p < 26; ++p) {
    ASSERT_TRUE(isalnum(txt[p]));
  }
}

static void test_ulids_waiting_between_them(ULID_Factory *uf, unsigned count,
                                            unsigned long wait_ms,
                                            int expected) {
  ULID last;
  for (unsigned p = 0; p < count; ++p) {
    if (wait_ms > 0) {
      struct timespec ts = {0, (long)wait_ms * 1000000};
      nanosleep(&ts, 0);
    }
    ULID ulid;
    ULID_Create(uf, &ulid);
    if (p > 0) {
      EXPECT_EQ(expected, ULID_Compare(&last, &ulid));
    }
    last = ulid;
  }
}

TEST(culid, default_without_sleeping_produces_sorted_ulids) {
  ULID_Factory uf;
  ULID_Factory_Init(&uf);

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, 0, -1);
}

TEST(culid, default_with_sleeping_produces_sorted_ulids) {
  ULID_Factory uf;
  ULID_Factory_Init(&uf);

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, MS_BETWEEN_ULIDS, -1);
}

TEST(culid, rand_without_sleeping_produces_sorted_ulids) {
  ULID_Factory uf;
  ULID_Factory_SetEntropyKind(&uf, ULID_ENTROPY_RAND);

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, 0, -1);
}

TEST(culid, rand_with_sleeping_produces_sorted_ulids) {
  ULID_Factory uf;
  ULID_Factory_SetEntropyKind(&uf, ULID_ENTROPY_RAND);

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, MS_BETWEEN_ULIDS, -1);
}

TEST(culid, entropy_seed_without_sleeping_produces_sorted_ulids) {
  ULID_Factory uf;
  ULID_Factory_SetEntropySeed(&uf, 19690721);

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, 0, -1);
}

TEST(culid, entropy_seed_with_sleeping_produces_sorted_ulids) {
  ULID_Factory uf;
  ULID_Factory_SetEntropySeed(&uf, 19690720); // go Neil!

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, MS_BETWEEN_ULIDS, -1);
}

TEST(culid, fixed_time_entropy_without_sleeping_produces_sorted_ulids) {
  ULID_Factory uf;
  uint8_t entropy[10] = {1, 2, 3, 4, 5, 6, 7, 8, 7, 6};
  ULID_Factory_SetEntropy(&uf, entropy);
  ULID_Factory_SetTime(&uf, 1733505202556);

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, 0, -1);
}

TEST(culid, fixed_time_entropy_with_sleeping_produces_sorted_ulids) {
  ULID_Factory uf;
  // clang-format off
  uint8_t entropy[10] = {
    0xde, 0xad, 0xbe, 0xef,
    0xc0, 0xff, 0xee, 0xba, 0xbe,
    0x11,
  };
  // clang-format on
  ULID_Factory_SetEntropy(&uf, entropy);
  ULID_Factory_SetTime(&uf, 1733505202556);

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, MS_BETWEEN_ULIDS, -1);
}

#if 0

TEST(Create, 1) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  ULID ulid1 = {0};
  ULID_Encode(1484581420, rnd, &ulid1);

  ULID ulid2 = ULID_Create(1484581420, rnd);

  ASSERT_EQ(0, ULID_Compare(&ulid1, &ulid2));
}

TEST(EncodeTimeNow, 1) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  ULID ulid = {0};
  ULID_EncodeTimeNow(&ulid);
  ULID_EncodeEntropy(rnd, &ulid);
  char txt[26];
  ULID_MarshalTo(&ulid, txt);

  for (unsigned p = 0; p < 26; ++p) {
    ASSERT_NE(nullptr, strchr(ULID_Encoding, txt[p]));
  }
}

TEST(EncodeTimeSystemClockNow, 1) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  ULID ulid = {0};
  ULID_EncodeTimeSystemClockNow(&ulid);
  ULID_EncodeEntropy(rnd, &ulid);
  char txt[26];
  ULID_MarshalTo(&ulid, txt);

  for (unsigned p = 0; p < 26; ++p) {
    ASSERT_NE(nullptr, strchr(ULID_Encoding, txt[p]));
  }
}

TEST(EncodeEntropyRand, 1) {
  ULID ulid = {0};
  ULID_EncodeTimeNow(&ulid);
  ULID_EncodeEntropyRand(&ulid);
  char txt[26];
  ULID_MarshalTo(&ulid, txt);

  for (unsigned p = 0; p < 26; ++p) {
    ASSERT_NE(nullptr, strchr(ULID_Encoding, txt[p]));
  }
}

TEST(EncodeEntropyRand, 2) {
  time_t timestamp = 1000000;
  unsigned long duration_s = timestamp;
  unsigned long duration_ms = duration_s * 1000;
  unsigned long duration_ns = duration_ms * 1000000;

  ULID ulid1 = {0};
  ULID_EncodeTime(duration_ms, &ulid1);

  std::srand(duration_ns);
  ULID_EncodeEntropyRand(&ulid1);

  ULID ulid2 = {0};
  ULID_EncodeTime(duration_ms, &ulid2);

  std::srand(duration_ns);
  ULID_EncodeEntropyRand(&ulid2);

  ASSERT_EQ(0, ULID_Compare(&ulid1, &ulid2));
}

TEST(EncodeNowRand, 1) {
  ULID ulid = {0};
  ULID_EncodeNowRand(&ulid);
  char txt[26];
  ULID_MarshalTo(&ulid, txt);

  for (unsigned p = 0; p < 26; ++p) {
    ASSERT_NE(nullptr, strchr(ULID_Encoding, txt[p]));
  }
}

TEST(CreateNowRand, 1) {
  ULID ulid = ULID_CreateNowRand();
  char txt[26];
  ULID_MarshalTo(&ulid, txt);

  for (unsigned p = 0; p < 26; ++p) {
    ASSERT_NE(nullptr, strchr(ULID_Encoding, txt[p]));
  }
}

TEST(MarshalBinary, 1) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  ULID ulid = ULID_Create(1484581420, rnd);
  uint8_t bin[16];
  ULID_MarshalBinaryTo(&ulid, bin);

#ifdef ULID_USE_UINT128
  for (int i = 15; i >= 0; i--) {
    ASSERT_EQ((uint8_t)ulid, bin[i]);
    ulid >>= 8;
  }
#else
  for (int i = 0; i < 16; i++) {
    ASSERT_EQ(ulid.data[i], bin[i]);
  }
#endif
}

TEST(Unmarshal, 1) {
  ULID ulid = {0};
  ULID_UnmarshalFrom("0001C7STHC0G2081040G208104", &ulid);
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  ULID ulid_expected = ULID_Create(1484581420, rnd);
  ASSERT_EQ(0, ULID_Compare(&ulid_expected, &ulid));
}

TEST(UnmarshalBinary, 1) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  ULID ulid_expected = ULID_Create(1484581420, rnd);
  uint8_t bin[16];
  ULID_MarshalBinaryTo(&ulid_expected, bin);
  ULID ulid = {0};
  ULID_UnmarshalBinaryFrom(bin, &ulid);
  ASSERT_EQ(0, ULID_Compare(&ulid_expected, &ulid));
}

TEST(Time, 1) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  ULID ulid = ULID_Create(1484581420, rnd);
  ASSERT_EQ(1484581420, ULID_Time(&ulid));
}

// https://github.com/oklog/ulid/blob/master/ulid_test.go#L160-L169
TEST(AlizainCompatibility, 1) {
  ULID ulid_got = {0};
  ULID_EncodeTime((uint64_t)1469918176385, &ulid_got);

  ULID ulid_want = {0};
  ULID_UnmarshalFrom("01ARYZ6S410000000000000000", &ulid_want);
  ASSERT_EQ(0, ULID_Compare(&ulid_want, &ulid_got));
}

TEST(LexicographicalOrder, 1) {
  ULID ulid1 = ULID_CreateNowRand();
  sleep(1);
  ULID ulid2 = ULID_CreateNowRand();

  EXPECT_EQ(-1, ULID_Compare(&ulid1, &ulid2));
  EXPECT_EQ(1, ULID_Compare(&ulid2, &ulid1));
}

#endif
