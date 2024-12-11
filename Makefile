first: all

NAME = culid

#
# Run like this to see all dynamic libraries being loaded:
#   $ DYLD_PRINT_LIBRARIES=1 DYLD_PRINT_LIBRARIES_POST_LAUNCH=1 DYLD_PRINT_RPATHS=1 ./culid
#

#
# Prefer this for development -- but there is an impact on performance.
#
# Using sanitizers sometimes causes this error:
#
#   culid(88471,0x1f9c58f40) malloc: nano zone abandoned due to inability to reserve vm space.
#
# The error can be silenced by setting envvar MallocNanoZone to 0:
#
#   $ MallocNanoZone=0 ./culid
#
# C_CPP_ALL_FLAGS += -fsanitize=undefined,address
# C_CPP_ALL_FLAGS += -g

# Prefer this for production -- optimized performance.
C_CPP_ALL_FLAGS += -O

C_CPP_COMPILE_FLAGS += -Wall -Wextra -Wshadow
C_CPP_COMPILE_FLAGS += -I.
C_CPP_COMPILE_FLAGS += -I$(HOMEBREW_PREFIX)/include

C_CPP_LINK_FLAGS += -L$(HOMEBREW_PREFIX)/lib

CFLAGS += -std=c11
CFLAGS += $(C_CPP_ALL_FLAGS)
CFLAGS += $(C_CPP_COMPILE_FLAGS)

CPP_FLAGS += -std=c++17
CPP_FLAGS += $(C_CPP_ALL_FLAGS)
CPP_FLAGS += $(C_CPP_COMPILE_FLAGS)

LDFLAGS += $(C_CPP_ALL_FLAGS)
LDFLAGS += $(C_CPP_LINK_FLAGS)

TEST_LIBS = gtest gtest_main
TEST_LINK = $(patsubst %,-l%,$(TEST_LIBS))

BENCH_LIBS = benchmark benchmark_main
BENCH_LINK = $(patsubst %,-l%,$(BENCH_LIBS))

EXE = $(NAME)
LIBRARY = lib$(NAME).a

C_SRC = \
	mtwister.c \
	ulid.c \

C_HDR = $(C_SRC:.c=.h)
C_OBJ = $(C_SRC:.c=.o)
EXE_OBJ = $(NAME).o


%.o: %.c $(C_HDR)
	cc $(CFLAGS) -c -o $@ $(word 1, $^)

$(LIBRARY): $(C_OBJ)  ## (re)build library
	ar -crs $@ $^

$(EXE): $(LIBRARY) $(EXE_OBJ)
	cc $(LDFLAGS) -o $@ $^

all: $(EXE)  ## build everything

clean:  ## clean up everything
	rm -f $(C_OBJ) $(EXE_OBJ)
	rm -f $(EXE) $(LIBRARY)
	rm -fr $(NAME).dSYM
	rm -fr t/ulid_test t/ulid_test.dSYM
	rm -fr t/ulid_bench t/ulid_bench.dSYM

help: ## display this help
	@grep -E '^[ a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?# "}; {printf "\033[36;1m%-30s\033[0m %s\n", $$1, $$2}'

.PHONY: first all test bench clean help

t/ulid_test: t/ulid_test.cc $(LIBRARY)
	c++ $(CPP_FLAGS) -o $@ $^ $(LDFLAGS) $(TEST_LINK)

t/ulid_bench: t/ulid_bench.cc $(LIBRARY)
	c++ $(CPP_FLAGS) -o $@ $^ $(LDFLAGS) $(BENCH_LINK)

test: t/ulid_test  ## run all tests
	t/ulid_test

bench: t/ulid_bench  ## run all benchmarks
	t/ulid_bench
