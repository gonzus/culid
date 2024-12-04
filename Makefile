first: all

HOMEBREW_DIR = /opt/homebrew

# AFLAGS += -DULID_FORCE_UINT128
AFLAGS += -DULID_FORCE_STRUCT
AFLAGS += -Wall -Wextra -Wshadow
AFLAGS += -I.
AFLAGS += -I$(HOMEBREW_DIR)/include
AFLAGS += -L$(HOMEBREW_DIR)/lib
AFLAGS += -g
# AFLAGS += -O

CFLAGS += -std=c11
CFLAGS += $(AFLAGS)

CPP_FLAGS += -std=c++17
CPP_FLAGS += $(AFLAGS)

TEST_LIBS = -lgtest -lgtest_main
BENCH_LIBS = -lbenchmark -lbenchmark_main

HEADERS = ulid.h ulid_uint128.h ulid_struct.h

gonzo: gonzo.c $(HEADERS)  ## build gonzo, a sample program
	cc $(CFLAGS) -o $@ $(word 1, $^)

t/ulid_test: t/ulid_test.cc $(HEADERS)
	c++ $(CPP_FLAGS) -o $@ $(word 1, $^) $(TEST_LIBS)

t/ulid_bench: t/ulid_bench.cc $(HEADERS)
	c++ $(CPP_FLAGS) -o $@ $(word 1, $^) $(BENCH_LIBS)

test: t/ulid_test  ## run all tests
	t/ulid_test

bench: t/ulid_bench  ## run all benchmarks
	t/ulid_bench

all: gonzo  ## build everything

clean:  ## clean up everything
	rm -fr gonzo gonzo.dSYM
	rm -fr t/ulid_test t/ulid_test.dSYM
	rm -fr t/ulid_bench t/ulid_bench.dSYM

help:  ## display this help
	@grep -E '^[ a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?# "}; {printf "\033[36;1m%-30s\033[0m %s\n", $$1, $$2}'

.PHONY: first all test bench clean help
