#define CULID_IMPLEMENTATION
#include <ulid.h>

#include <benchmark/benchmark.h>
#include <cstring>

static void CreateDefault(benchmark::State &state) {
  ULID_Factory uf;
  ULID_Factory_Default(&uf);
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

static void Format(benchmark::State &state) {
  ULID_Factory uf;
  ULID_Factory_Default(&uf);
  ULID ulid;
  ULID_Create(&uf, &ulid);
  while (state.KeepRunning()) {
    char txt[26];
    ULID_Format(&ulid, txt);
  }
}
BENCHMARK(Format);

static void Parse(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    ULID_Parse(&ulid, "0001C7STHC0G2081040G208104");
  }
}
BENCHMARK(Parse);

static void Compare(benchmark::State &state) {
  ULID_Factory uf;
  ULID_Factory_Default(&uf);
  ULID l, r;
  ULID_Create(&uf, &l);
  ULID_Create(&uf, &r);
  while (state.KeepRunning()) {
    ULID_Compare(&l, &r);
  }
}
BENCHMARK(Compare);

BENCHMARK_MAIN();
