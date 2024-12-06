#include "ulid.h"
#include <stdio.h>
#include <stdlib.h>

static void print_ulids(ULID_Factory *uf, unsigned n) {
  for (unsigned p = 0; p < n; ++p) {
    ULID ulid;
    ULID_Create(uf, &ulid);
    char txt[26];
    ULID_Format(&ulid, txt);
    printf("ULID: [%.*s]\n", 26, txt);
  }
}

int main(int argc, char *argv[]) {
  ULID_Factory uf;
#if 1
  // Random entropy, current time
  ULID_Factory_Init(&uf);
#elif 0
  // Fixed entropy, fixed time
  uint8_t entropy[10] = {1, 2, 3, 4, 5, 6, 7, 8, 7, 6};
  ULID_Factory_SetEntropy(&uf, entropy);
  ULID_Factory_SetTime(&uf, 1733505202556);
#else
  // Seed for entropy, fixed time
  ULID_Factory_SetEntropySeed(&uf, 19690721);
  ULID_Factory_SetTime(&uf, 1733505202556);
#endif
  if (argc <= 1) {
    print_ulids(&uf, 1);
  } else {
    for (int p = 1; p < argc; ++p) {
      unsigned n = atoi(argv[p]);
      print_ulids(&uf, n);
    }
  }
  return 0;
}
