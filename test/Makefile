CC = gcc
LIB = -lcmocka

SRC = ../src

_test_buffer: test_buffer.c $(SRC)/buffer.c
	$(CC) -o $@ $(LIB) $^

.PHONY: clean

clean:
	rm _test_*
