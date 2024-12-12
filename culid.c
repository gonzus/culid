#define CULID_IMPLEMENTATION
#include "ulid.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static void print_ulids(ULID_Factory *uf, unsigned n);
static void show_help(const char *prog);
static uint8_t get_byte(const char *txt, unsigned *pos);

int main(int argc, char *argv[]) {
  static struct option long_options[] = {
      {"rand", no_argument, 0, 'r'},
      {"seed", required_argument, 0, 's'},
      {"entropy", required_argument, 0, 'e'},
      {"time", required_argument, 0, 't'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0},
  };
  const char *prog = argv[0];
  ULID_Factory uf;
  ULID_Factory_Default(&uf);

  int option = 0;
  while ((option = getopt_long(argc, argv, ":rs:e:t:h", long_options, 0)) !=
         -1) {
    switch (option) {
    case 'r':
#if 0
      printf("Using entropy from rand/srand\n");
#endif
      ULID_Factory_SetEntropyKind(&uf, ULID_ENTROPY_RAND);
      break;
    case 's': {
      uint32_t seed = atoi(optarg);
#if 0
      printf("Using seed [%u]\n", seed);
#endif
      ULID_Factory_SetEntropySeed(&uf, seed);
      break;
    }
    case 'e': {
      unsigned pos = 0;
      uint8_t entropy[ULID_BYTES_ENTROPY] = {0};
      for (unsigned e = 0; e < ULID_BYTES_ENTROPY; ++e) {
        entropy[e] = get_byte(optarg, &pos);
      }
#if 0
      printf("Using given entropy");
      for (unsigned e = 0; e < ULID_BYTES_ENTROPY; ++e) {
        unsigned v = entropy[e];
        printf(" %02x", v);
      }
      printf("\n");
#endif
      ULID_Factory_SetEntropy(&uf, entropy);
      break;
    }
    case 't':
#if 0
      printf("Using time [%s]\n", optarg);
#endif
      ULID_Factory_SetTime(&uf, atoi(optarg));
      break;
    case 'h':
      show_help(prog);
      return 1;
    case ':':
      fprintf(stderr, "ERROR: missing argument for option %s\n\n",
              argv[optind - 1]);
      show_help(prog);
      return 1;
    case '?':
    default:
      fprintf(stderr, "ERROR: unknown option %s\n\n", argv[optind - 1]);
      show_help(prog);
      return 1;
    }
  }
  argc -= optind;
  argv += optind;
  if (argc <= 0) {
    print_ulids(&uf, 1);
  } else {
    for (int p = 0; p < argc; ++p) {
      unsigned n = atoi(argv[p]);
      print_ulids(&uf, n);
    }
  }
  return 0;
}

static void print_ulids(ULID_Factory *uf, unsigned n) {
  for (unsigned p = 0; p < n; ++p) {
    ULID ulid;
    ULID_Create(uf, &ulid);
    char txt[ULID_BYTES_FORMATTED];
    ULID_Format(&ulid, txt);
    printf("ULID: [%.*s]\n", ULID_BYTES_FORMATTED, txt);
  }
}

static void show_help(const char *prog) {
  fprintf(stderr, "%s -- Utility to generate ULIDs\n", prog);
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage: %s [options] number...\n", prog);
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  --rand        | -r  use rand / srand for entropy "
                  "(default: use Mersenne Twister)\n");
  fprintf(stderr, "  --seed ...    | -s  use specified seed for entropy "
                  "(default: use random seed)\n");
  fprintf(stderr, "  --entropy ... | -e  use specified values for entropy "
                  "(default: use random entropy)\n");
  fprintf(stderr, "  --time ...    | -t  use specified time (in ms) "
                  "(default: use current time)\n");
  fprintf(stderr, "  --help        | -h  show this help\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Examples:\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  # generate 1 ULID\n");
  fprintf(stderr, "  %s\n", prog);
  fprintf(stderr, "\n");
  fprintf(stderr, "  # generate 9 ULIDs using rand / srand for entropy\n");
  fprintf(stderr, "  %s --rand 9\n", prog);
  fprintf(stderr, "\n");
  fprintf(stderr, "  # generate 9 ULIDs using a given seed for entropy\n");
  fprintf(stderr, "  %s --seed 12345 9\n", prog);
  fprintf(stderr, "\n");
  fprintf(stderr, "  # generate 9 ULIDs using specified values for entropy\n");
  fprintf(stderr, "  %s --entropy deadbeefc0ffeebabe11 9\n", prog);
  fprintf(stderr, "\n");
  fprintf(stderr, "  # generate 9 ULIDs using specified time (in ms)\n");
  fprintf(stderr, "  %s --time 3344556677 9\n", prog);
}

static unsigned h2d(char c) {
  if (c >= '0' && c <= '9')
    return c - '0' + 0;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return (unsigned)-1;
}

static uint8_t get_byte(const char *txt, unsigned *pos) {
  unsigned byte = 0;
  unsigned p = 0;
  while (p < 2) {
    char c = txt[(*pos)++];
    if (c == '\0')
      return 0;
    unsigned d = h2d(c);
    if (d == (unsigned)-1)
      continue; // skip separators: de:ad be+ef == deadbeef
    byte <<= 4;
    byte |= d;
    ++p;
  }
  return byte;
}
