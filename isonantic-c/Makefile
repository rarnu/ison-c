# Makefile for isonantic-c

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS =

SRCS = src/value.c src/schema.c src/complex.c
OBJS = $(SRCS:.c=.o)
TARGET = bin/libisonantic.a
TEST_BIN = bin/test_runner

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p bin
	ar rcs $@ $^

%.o: %.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o bin/$(notdir $@)

clean:
	rm -rf bin

test: $(TARGET)
	$(CC) $(CFLAGS) tests/test_runner.c -Lbin -lisonantic -o $(TEST_BIN)
	./$(TEST_BIN)

run_tests: test
