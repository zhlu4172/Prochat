CC=gcc
CFLAGS=-Wall -Werror -Wvla -std=gnu11 -fsanitize=address
PFLAGS=-fprofile-arcs -ftest-coverage
DFLAGS=-g
HEADERS=server.h
SRC=server.c


procchat: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -o $@

test: tests/client1 tests/client1_1 tests/client1_2 tests/client1_2_1 tests/client2_1 tests/client2_2 server
	bash test.sh


clean:
	rm -f procchat
	rm -f test

