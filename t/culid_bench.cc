#include <benchmark/benchmark.h>

#include <cstring>
#include <culid.h>

static void EncodeTime(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    EncodeTime(1484581420, &ulid);
  }
}
BENCHMARK(EncodeTime);

static void EncodeTimeNow(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    EncodeTimeNow(&ulid);
  }
}
BENCHMARK(EncodeTimeNow);

static void EncodeTimeSystemClockNow(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    EncodeTimeSystemClockNow(&ulid);
  }
}
BENCHMARK(EncodeTimeSystemClockNow);

static void EncodeEntropy(benchmark::State &state) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid = {0};
  while (state.KeepRunning()) {
    EncodeEntropy(rng, &ulid);
  }
}
BENCHMARK(EncodeEntropy);

static void EncodeEntropyRand(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    EncodeEntropyRand(&ulid);
  }
}
BENCHMARK(EncodeEntropyRand);

#if 0
static void EncodeEntropyMt19937(benchmark::State &state) {
  ulid::ULID ulid;
  std::mt19937 gen(4);

  while (state.KeepRunning()) {
    ulid::EncodeEntropyMt19937(gen, ulid);
  }
}
BENCHMARK(EncodeEntropyMt19937);
#endif

static void Encode(benchmark::State &state) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  ULID ulid = {0};
  while (state.KeepRunning()) {
    Encode(1484581420, rng, &ulid);
  }
}
BENCHMARK(Encode);

static void EncodeNowRand(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    EncodeNowRand(&ulid);
  }
}
BENCHMARK(EncodeNowRand);

static void Create(benchmark::State &state) {
  uint8_t rng[10];
  for (unsigned p = 0; p < 10; ++p) {
    rng[p] = 4;
  }
  while (state.KeepRunning()) {
    Create(1484581420, rng);
  }
}
BENCHMARK(Create);

static void CreateNowRand(benchmark::State &state) {
  while (state.KeepRunning()) {
    CreateNowRand();
  }
}
BENCHMARK(CreateNowRand);

static void MarshalTo(benchmark::State &state) {
  char a[27];
  ULID ulid = CreateNowRand();
  while (state.KeepRunning()) {
    MarshalTo(&ulid, a);
  }
}
BENCHMARK(MarshalTo);

#if 0
static void Marshal(benchmark::State &state) {
  ulid::ULID ulid = ulid::CreateNowRand();
  while (state.KeepRunning()) {
    std::string m = ulid::Marshal(ulid);
  }
}
BENCHMARK(Marshal);
#endif

static void MarshalBinaryTo(benchmark::State &state) {
  uint8_t a[16];
  ULID ulid = CreateNowRand();
  while (state.KeepRunning()) {
    MarshalBinaryTo(&ulid, a);
  }
}
BENCHMARK(MarshalBinaryTo);

#if 0
static void MarshalBinary(benchmark::State &state) {
  ulid::ULID ulid = ulid::CreateNowRand();
  while (state.KeepRunning()) {
    std::vector<uint8_t> m = ulid::MarshalBinary(ulid);
  }
}
BENCHMARK(MarshalBinary);
#endif

static void UnmarshalFrom(benchmark::State &state) {
  ULID ulid = {0};
  while (state.KeepRunning()) {
    UnmarshalFrom("0001C7STHC0G2081040G208104", &ulid);
  }
}
BENCHMARK(UnmarshalFrom);

#if 0
static void Unmarshal(benchmark::State &state) {
  while (state.KeepRunning()) {
    ulid::ULID ulid = ulid::Unmarshal("0001C7STHC0G2081040G208104");
  }
}
BENCHMARK(Unmarshal);
#endif

static void UnmarshalBinaryFrom(benchmark::State &state) {
  ULID ulid = {0};
  uint8_t b[16] = {0x1,  0xc0, 0x73, 0x62, 0x4a, 0xaf, 0x39, 0x78,
                   0x51, 0x4e, 0xf8, 0x44, 0x3b, 0xb2, 0xa8, 0x59};
  while (state.KeepRunning()) {
    UnmarshalBinaryFrom(b, &ulid);
  }
}
BENCHMARK(UnmarshalBinaryFrom);

#if 0
static void UnmarshalBinary(benchmark::State &state) {
  while (state.KeepRunning()) {
    ulid::ULID ulid =
        ulid::UnmarshalBinary({0x1, 0xc0, 0x73, 0x62, 0x4a, 0xaf, 0x39, 0x78,
                               0x51, 0x4e, 0xf8, 0x44, 0x3b, 0xb2, 0xa8, 0x59});
  }
}
BENCHMARK(UnmarshalBinary);
#endif

static void Time(benchmark::State &state) {
  ULID ulid = CreateNowRand();
  while (state.KeepRunning()) {
    Time(&ulid);
  }
}
BENCHMARK(Time);

static void CompareULIDs(benchmark::State &state) {
  ULID ulid1 = CreateNowRand();
  ULID ulid2 = ulid1;
  while (state.KeepRunning()) {
    CompareULIDs(&ulid1, &ulid2);
  }
}
BENCHMARK(CompareULIDs);

BENCHMARK_MAIN();
