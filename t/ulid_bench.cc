#include <benchmark/benchmark.h>

#include <cstring>
#include <ulid.h>

static void EncodeTime(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    ULID_EncodeTime(1484581420, &ulid);
  }
}
BENCHMARK(EncodeTime);

static void EncodeTimeNow(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    ULID_EncodeTimeNow(&ulid);
  }
}
BENCHMARK(EncodeTimeNow);

static void EncodeTimeSystemClockNow(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    ULID_EncodeTimeSystemClockNow(&ulid);
  }
}
BENCHMARK(EncodeTimeSystemClockNow);

static void EncodeEntropy(benchmark::State &state) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  ULID ulid = {0};
  while (state.KeepRunning()) {
    ULID_EncodeEntropy(rnd, &ulid);
  }
}
BENCHMARK(EncodeEntropy);

static void EncodeEntropyRand(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    ULID_EncodeEntropyRand(&ulid);
  }
}
BENCHMARK(EncodeEntropyRand);

static void Encode(benchmark::State &state) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  ULID ulid = {0};
  while (state.KeepRunning()) {
    ULID_Encode(1484581420, rnd, &ulid);
  }
}
BENCHMARK(Encode);

static void EncodeNowRand(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    ULID_EncodeNowRand(&ulid);
  }
}
BENCHMARK(EncodeNowRand);

static void Create(benchmark::State &state) {
  uint8_t rnd[10];
  for (unsigned p = 0; p < 10; ++p) {
    rnd[p] = 4;
  }
  while (state.KeepRunning()) {
    ULID_Create(1484581420, rnd);
  }
}
BENCHMARK(Create);

static void CreateNowRand(benchmark::State &state) {
  while (state.KeepRunning()) {
    ULID_CreateNowRand();
  }
}
BENCHMARK(CreateNowRand);

static void MarshalTo(benchmark::State &state) {
  char a[26];
  ULID ulid = ULID_CreateNowRand();
  while (state.KeepRunning()) {
    ULID_MarshalTo(&ulid, a);
  }
}
BENCHMARK(MarshalTo);

static void MarshalBinaryTo(benchmark::State &state) {
  uint8_t a[16];
  ULID ulid = ULID_CreateNowRand();
  while (state.KeepRunning()) {
    ULID_MarshalBinaryTo(&ulid, a);
  }
}
BENCHMARK(MarshalBinaryTo);

static void UnmarshalFrom(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    ULID_UnmarshalFrom("0001C7STHC0G2081040G208104", &ulid);
  }
}
BENCHMARK(UnmarshalFrom);

static void UnmarshalBinaryFrom(benchmark::State &state) {
  ULID ulid = {0};
  // clang-format off
  uint8_t b[16] = {
    0x01, 0xc0, 0x73, 0x62, 0x4a, 0xaf, 0x39, 0x78,
    0x51, 0x4e, 0xf8, 0x44, 0x3b, 0xb2, 0xa8, 0x59,
  };
  // clang-format on
  while (state.KeepRunning()) {
    ULID_UnmarshalBinaryFrom(b, &ulid);
  }
}
BENCHMARK(UnmarshalBinaryFrom);

static void Time(benchmark::State &state) {
  ULID ulid = ULID_CreateNowRand();
  while (state.KeepRunning()) {
    ULID_Time(&ulid);
  }
}
BENCHMARK(Time);

static void CompareULIDs(benchmark::State &state) {
  ULID ulid1 = ULID_CreateNowRand();
  ULID ulid2 = ulid1;
  while (state.KeepRunning()) {
    ULID_Compare(&ulid1, &ulid2);
  }
}
BENCHMARK(CompareULIDs);

BENCHMARK_MAIN();
