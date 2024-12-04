#include <gtest/gtest.h>

#include <cstring>
#include <culid.h>

TEST(basic, 1) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid = Create(time(0), rng);

  char txt[27];
  MarshalTo(&ulid, txt);

  ASSERT_EQ(26ul, strlen(txt));
  for (unsigned p = 0; p < 27; ++p) {
    ASSERT_NE(nullptr, strchr(Encoding, txt[p]));
  }
}

TEST(Create, 1) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid1 = {0};
  Encode(1484581420, rng, &ulid1);

  ULID ulid2 = Create(1484581420, rng);

  ASSERT_EQ(0, CompareULIDs(&ulid1, &ulid2));
}

TEST(EncodeTimeNow, 1) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid = {0};
  EncodeTimeNow(&ulid);
  EncodeEntropy(rng, &ulid);
  char txt[27];
  MarshalTo(&ulid, txt);

  ASSERT_EQ(26ul, strlen(txt));
  for (unsigned p = 0; p < 27; ++p) {
    ASSERT_NE(nullptr, strchr(Encoding, txt[p]));
  }
}

TEST(EncodeTimeSystemClockNow, 1) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid = {0};
  EncodeTimeSystemClockNow(&ulid);
  EncodeEntropy(rng, &ulid);
  char txt[27];
  MarshalTo(&ulid, txt);

  ASSERT_EQ(26ul, strlen(txt));
  for (unsigned p = 0; p < 27; ++p) {
    ASSERT_NE(nullptr, strchr(Encoding, txt[p]));
  }
}

TEST(EncodeEntropyRand, 1) {
  ULID ulid = {0};
  EncodeTimeNow(&ulid);
  EncodeEntropyRand(&ulid);
  char txt[27];
  MarshalTo(&ulid, txt);

  ASSERT_EQ(26ul, strlen(txt));
  for (unsigned p = 0; p < 27; ++p) {
    ASSERT_NE(nullptr, strchr(Encoding, txt[p]));
  }
}

TEST(EncodeEntropyRand, 2) {
  time_t timestamp = 1000000;
  unsigned long duration_s = timestamp;
  unsigned long duration_ms = duration_s * 1000;
  unsigned long duration_ns = duration_ms * 1000000;

  ULID ulid1 = {0};
  EncodeTime(duration_ms, &ulid1);

  std::srand(duration_ns);
  EncodeEntropyRand(&ulid1);

  ULID ulid2 = {0};
  EncodeTime(duration_ms, &ulid2);

  std::srand(duration_ns);
  EncodeEntropyRand(&ulid2);

  ASSERT_EQ(0, CompareULIDs(&ulid1, &ulid2));
}

#if 0
TEST(EncodeEntropyMt19937, 1) {
  ulid::ULID ulid = 0;
  ulid::EncodeTimeNow(ulid);

  std::mt19937 generator(4);
  ulid::EncodeEntropyMt19937(generator, ulid);

  std::string str = ulid::Marshal(ulid);
  ASSERT_EQ(26, str.size());
  for (char c : str) {
    ASSERT_NE(std::string::npos, std::string(ulid::Encoding).find(c));
  }
}
#endif

TEST(EncodeNowRand, 1) {
  ULID ulid = {0};
  EncodeNowRand(&ulid);
  char txt[27];
  MarshalTo(&ulid, txt);

  ASSERT_EQ(26ul, strlen(txt));
  for (unsigned p = 0; p < 27; ++p) {
    ASSERT_NE(nullptr, strchr(Encoding, txt[p]));
  }
}

TEST(CreateNowRand, 1) {
  ULID ulid = CreateNowRand();
  char txt[27];
  MarshalTo(&ulid, txt);

  ASSERT_EQ(26ul, strlen(txt));
  for (unsigned p = 0; p < 27; ++p) {
    ASSERT_NE(nullptr, strchr(Encoding, txt[p]));
  }
}

TEST(MarshalBinary, 1) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid = Create(1484581420, rng);
  uint8_t bin[16];
  MarshalBinaryTo(&ulid, bin);

#ifdef CULID_USE_UINT128
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
  UnmarshalFrom("0001C7STHC0G2081040G208104", &ulid);
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid_expected = Create(1484581420, rng);
  ASSERT_EQ(0, CompareULIDs(&ulid_expected, &ulid));
}

TEST(UnmarshalBinary, 1) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid_expected = Create(1484581420, rng);
  uint8_t bin[16];
  MarshalBinaryTo(&ulid_expected, bin);
  ULID ulid = {0};
  UnmarshalBinaryFrom(bin, &ulid);
  ASSERT_EQ(0, CompareULIDs(&ulid_expected, &ulid));
}

TEST(Time, 1) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid = Create(1484581420, rng);
  ASSERT_EQ(1484581420, Time(&ulid));
}

// https://github.com/oklog/ulid/blob/master/ulid_test.go#L160-L169
TEST(AlizainCompatibility, 1) {
  ULID ulid_got = {0};
  EncodeTime((uint64_t)1469918176385, &ulid_got);

  ULID ulid_want = {0};
  UnmarshalFrom("01ARYZ6S410000000000000000", &ulid_want);
  ASSERT_EQ(0, CompareULIDs(&ulid_want, &ulid_got));
}

TEST(LexicographicalOrder, 1) {
  ULID ulid1 = CreateNowRand();
  sleep(1);
  ULID ulid2 = CreateNowRand();

  EXPECT_EQ(-1, CompareULIDs(&ulid1, &ulid2));
  EXPECT_EQ(1, CompareULIDs(&ulid2, &ulid1));
}
