HOMEBREW_DIR = /opt/homebrew

CFLAGS += -std=c11
CFLAGS += -DCULID_FORCE_UINT128
# CFLAGS += -DCULID_FORCE_STRUCT
CFLAGS += -Wall -Wextra -Wshadow
CFLAGS += -g
# CFLAGS += -O

CPP_FLAGS += -std=c++17
CPP_FLAGS += -I$(HOMEBREW_DIR)/include -I.
CPP_FLAGS += -L$(HOMEBREW_DIR)/lib
CPP_LIBS = -lgtest -lgtest_main

gonzo: gonzo.c culid.h
	cc $(CFLAGS) -o gonzo gonzo.c

t/culid_test: t/culid_test.cc
	c++ $(CPP_FLAGS) -o t/culid_test t/culid_test.cc $(CPP_LIBS)

test: t/culid_test
	t/culid_test

clean:
	rm -fr gonzo gonzo.dSYM
	rm -f t/culid_test
