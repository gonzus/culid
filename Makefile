gonzo: gonzo.o
	cc -std=c11 -Wall -Wextra -Wshadow -g -o gonzo gonzo.o

gonzo.o: gonzo.c culid.h
	cc -c -std=c11 -DCULID_FORCE_UINT128 -Wall -Wextra -Wshadow -g -o gonzo.o gonzo.c
	# cc -c -std=c11 -DCULID_FORCE_STRUCT -Wall -Wextra -Wshadow -g -o gonzo.o gonzo.c

t/culid_test: t/culid_test.cc
	c++ -std=c++17 -I/opt/homebrew/include -I. -o t/culid_test t/culid_test.cc -L/opt/homebrew/lib -lgtest -lgtest_main

test: t/culid_test
	t/culid_test

clean:
	rm -f gonzo gonzo.o
	rm -f t/culid_test
