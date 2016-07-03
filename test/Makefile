CC = gcc
LIB = -lcmocka
INC = ../include

SRC = ../src

HTTP_SRC = $(SRC)/http/status.c $(SRC)/http/header.c \
           $(SRC)/http/method.c $(SRC)/http/version.c \
		   $(SRC)/http/misc.c $(SRC)/http/connection.c \
		   $(SRC)/http/codec.c

all: TEST_buffer TEST_coroutine TEST_slinklist TEST_dlinklist TEST_hashmap TEST_ringlist TEST_arraylist TEST_httpcodec

TEST_httpcodec: test_httpcodec.c $(HTTP_SRC)
	$(CC) -o $@ $(LIB) -I$(INC) $^

TEST_buffer: test_buffer.c $(INC)/buffer.h
	$(CC) -o $@ $(LIB) -I$(INC) $^

TEST_coroutine: test_coroutine.c $(SRC)/coroutine.c $(SRC)/coroutine.S
	$(CC) -o $@ $(LIB) -I$(INC) $^

TEST_slinklist: test_slinklist.c $(SRC)/util/list.c
	$(CC) -o $@ $(LIB) -I$(INC) $^

TEST_dlinklist: test_dlinklist.c $(SRC)/util/list.c
	$(CC) -o $@ $(LIB) -I$(INC) $^

TEST_hashmap: test_hashmap.c $(SRC)/util/map.c
	$(CC) -o $@ $(LIB) -I$(INC) $^ -g

TEST_ringlist: test_ringlist.c $(SRC)/util/list.c
	$(CC) -o $@ $(LIB) -I$(INC) $^

TEST_arraylist: test_arraylist.c $(SRC)/util/list.c
	$(CC) -o $@ $(LIB) -I$(INC) $^

.PHONY: clean
clean:
	rm TEST_*
