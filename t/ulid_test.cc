#include <gtest/gtest.h>

#include <cstring>
#include <ctime>
#include <ulid.h>

enum {
  NUMBER_OF_ULIDS = 500,
  MS_BETWEEN_ULIDS = 1,
  TIME_MS = 1733505202556,
};

TEST(culid, basic_creation_and_formatting) {
  ULID_Factory uf;
  ULID_Factory_Default(&uf);

  ULID ulid;
  ULID_Create(&uf, &ulid);

  char txt[26];
  ULID_Format(&ulid, txt);

  for (unsigned p = 0; p < 26; ++p) {
    EXPECT_TRUE(isalnum(txt[p]));
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
  ULID_Factory_Default(&uf);

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, 0, -1);
}

TEST(culid, default_with_sleeping_produces_sorted_ulids) {
  ULID_Factory uf;
  ULID_Factory_Default(&uf);

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
  ULID_Factory_SetTime(&uf, TIME_MS);

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
  ULID_Factory_SetTime(&uf, TIME_MS);

  test_ulids_waiting_between_them(&uf, NUMBER_OF_ULIDS, MS_BETWEEN_ULIDS, -1);
}

TEST(culid, can_roundtrip_time_and_entropy) {
  ULID_Factory uf;
  // clang-format off
  uint8_t entropy[10] = {
    0xde, 0xad, 0xbe, 0xef,
    0xc0, 0xff, 0xee, 0xba, 0xbe,
    0x11,
  };
  // clang-format on
  ULID_Factory_SetEntropy(&uf, entropy);
  ULID_Factory_SetTime(&uf, TIME_MS);

  ULID ulid;
  ULID_Create(&uf, &ulid);

  unsigned long got_time_ms = 0;
  ULID_GetTime(&ulid, &got_time_ms);
  EXPECT_EQ(got_time_ms, TIME_MS);

  uint8_t got_entropy[ULID_BYTES_ENTROPY];

  ULID_GetEntropy(&ulid, got_entropy);
  EXPECT_EQ(entropy[0], got_entropy[0]);
  EXPECT_EQ(entropy[1], got_entropy[1]);
  EXPECT_EQ(entropy[2], got_entropy[2]);
  EXPECT_EQ(entropy[3], got_entropy[3]);
  EXPECT_EQ(entropy[4], got_entropy[4]);
  EXPECT_EQ(entropy[5], got_entropy[5]);
  EXPECT_EQ(entropy[6], got_entropy[6]);
  EXPECT_EQ(entropy[7], got_entropy[7]);
  EXPECT_EQ(entropy[8], got_entropy[8]);

  // last byte was incremented to preserver order
  EXPECT_EQ(entropy[9], got_entropy[9] - 1);
}
