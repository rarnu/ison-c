CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -O2
LDFLAGS = 

SRCDIR = src
OBJDIR = obj
BINDIR = bin
TESTDIR = tests

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

LIBRARY = $(BINDIR)/libison.a
TEST_BIN = $(BINDIR)/test_ison

.PHONY: all clean test

all: $(LIBRARY)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(OBJECTS) | $(BINDIR)
	ar rcs $@ $^

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TESTDIR)/advanced_tests.c $(LIBRARY) | $(BINDIR)
	$(CC) $(CFLAGS) $< -L$(BINDIR) -lison -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

install: $(LIBRARY)
	cp include/ison.h /usr/local/include/
	cp $(LIBRARY) /usr/local/lib/

uninstall:
	rm -f /usr/local/include/ison.h
	rm -f /usr/local/lib/libison.a
