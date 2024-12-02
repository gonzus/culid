gonzo: gonzo.o
	cc -std=c11 -Wall -Wextra -Wshadow -g -o gonzo gonzo.o

gonzo.o: gonzo.c culid.h
	cc -c -std=c11 -Wall -Wextra -Wshadow -g -o gonzo.o gonzo.c

clean:
	rm -f gonzo gonzo.o
