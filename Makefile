CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -Werror -fstack-protector-strong -g

.PHONY: all clean test

SRCDIR = src
BUILDDIR = build
BINARIES = $(BUILDDIR)/compile $(BUILDDIR)/interpret

all: $(BINARIES)

$(BUILDDIR)/compile: $(SRCDIR)/compiler.c $(BUILDDIR)/common.o
	$(CC) $(CFLAGS) -c $< -o $(BUILDDIR)/compile.o
	$(CC) -o $@ $(BUILDDIR)/compile.o $(BUILDDIR)/common.o

$(BUILDDIR)/interpret: $(SRCDIR)/interpreter.c $(BUILDDIR)/common.o
	$(CC) $(CFLAGS) -c $< -o $(BUILDDIR)/interpret.o
	$(CC) -o $@ $(BUILDDIR)/interpret.o $(BUILDDIR)/common.o

$(BUILDDIR)/common.o: src/common.c src/common.h
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	find $(BUILDDIR) -name "*.o" -delete
	find tests -name "*.myout" -delete

test: $(BINARIES) test.sh tests
	./test.sh
