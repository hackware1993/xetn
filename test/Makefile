CC = gcc
LIB = -lcmocka

SRC = ../src

all: _test_buffer _test_coroutine

_test_buffer: test_buffer.c $(SRC)/buffer.c
	$(CC) -o $@ $(LIB) $^

_test_coroutine: test_coroutine.c $(SRC)/coroutine.c $(SRC)/md.S
	$(CC) -o $@ $(LIB) $^

.PHONY: clean
clean:
	rm _test_*
