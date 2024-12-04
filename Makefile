HOMEBREW_DIR = /opt/homebrew

# AFLAGS += -DCULID_FORCE_UINT128
AFLAGS += -DCULID_FORCE_STRUCT
AFLAGS += -Wall -Wextra -Wshadow
AFLAGS += -I$(HOMEBREW_DIR)/include -I.
AFLAGS += -L$(HOMEBREW_DIR)/lib
AFLAGS += -g
# AFLAGS += -O

CFLAGS += -std=c11
CFLAGS += $(AFLAGS)

CPP_FLAGS += -std=c++17
CPP_FLAGS += $(AFLAGS)

TEST_LIBS = -lgtest -lgtest_main
BENCH_LIBS = -lbenchmark -lbenchmark_main

gonzo: gonzo.c culid.h culid_uint128.h culid_struct.h
	cc $(CFLAGS) -o gonzo gonzo.c

t/culid_test: t/culid_test.cc culid.h culid_uint128.h culid_struct.h
	c++ $(CPP_FLAGS) -o t/culid_test t/culid_test.cc $(TEST_LIBS)

t/culid_bench: t/culid_bench.cc culid.h culid_uint128.h culid_struct.h
	c++ $(CPP_FLAGS) -o t/culid_bench t/culid_bench.cc $(BENCH_LIBS)

test: t/culid_test
	t/culid_test

bench: t/culid_bench
	t/culid_bench

clean:
	rm -fr gonzo gonzo.dSYM
	rm -fr t/culid_test t/culid_test.dSYM
	rm -fr t/culid_bench t/culid_bench.dSYM
