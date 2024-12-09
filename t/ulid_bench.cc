#include <benchmark/benchmark.h>

#include <cstring>
#include <ulid.h>

static void CreateDefault(benchmark::State &state) {
  ULID_Factory uf;
  ULID_Factory_Init(&uf);
  while (state.KeepRunning()) {
    ULID ulid;
    ULID_Create(&uf, &ulid);
  }
}
BENCHMARK(CreateDefault);

static void CreateRandTOD(benchmark::State &state) {
  ULID_Factory uf;
  ULID_Factory_SetEntropyKind(&uf, ULID_ENTROPY_RAND);
  while (state.KeepRunning()) {
    ULID ulid;
    ULID_Create(&uf, &ulid);
  }
}
BENCHMARK(CreateRandTOD);

static void CreateRandSeedTOD(benchmark::State &state) {
  ULID_Factory uf;
  ULID_Factory_SetEntropyKind(&uf, ULID_ENTROPY_RAND);
  ULID_Factory_SetEntropySeed(&uf, 19690720);
  while (state.KeepRunning()) {
    ULID ulid;
    ULID_Create(&uf, &ulid);
  }
}
BENCHMARK(CreateRandSeedTOD);

static void CreateMTwisterTOD(benchmark::State &state) {
  ULID_Factory uf;
  ULID_Factory_SetEntropyKind(&uf, ULID_ENTROPY_MERSENNE_TWISTER);
  while (state.KeepRunning()) {
    ULID ulid;
    ULID_Create(&uf, &ulid);
  }
}
BENCHMARK(CreateMTwisterTOD);

static void CreateMTwisterSeedTOD(benchmark::State &state) {
  ULID_Factory uf;
  ULID_Factory_SetEntropyKind(&uf, ULID_ENTROPY_MERSENNE_TWISTER);
  ULID_Factory_SetEntropySeed(&uf, 19690720);
  while (state.KeepRunning()) {
    ULID ulid;
    ULID_Create(&uf, &ulid);
  }
}
BENCHMARK(CreateMTwisterSeedTOD);

#if 0
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
#endif

BENCHMARK_MAIN();
