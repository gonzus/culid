#include "culid.h"
#include <stdio.h>

static void print_ulids(unsigned n) {
  for (unsigned p = 0; p < n; ++p) {
    ULID ulid = CreateNowRand();
    char txt[27];
    MarshalTo(&ulid, txt);
    printf("ULID: [%s]\n", txt);
  }
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    print_ulids(1);
  } else {
    for (int p = 1; p < argc; ++p) {
      unsigned n = atoi(argv[p]);
      print_ulids(n);
    }
  }
  return 0;
}
