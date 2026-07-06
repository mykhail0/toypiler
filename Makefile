CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -Werror -fstack-protector-strong -g

.PHONY: all clean # test

SRCDIR = src
BUILDDIR = build
BINARIES = $(BUILDDIR)/compile $(BUILDDIR)/interpret

all: $(BINARIES)

build/compile: $(SRCDIR)/compiler.c $(BUILDDIR)/common.o
	$(CC) $(CFLAGS) -c $< -o $(BUILDDIR)/compile.o
	$(CC) -o $@ $(BUILDDIR)/compile.o $(BUILDDIR)/common.o

build/interpret: $(SRCDIR)/interpreter.c $(BUILDDIR)/common.o
	$(CC) $(CFLAGS) -c $< -o $(BUILDDIR)/interpret.o
	$(CC) -o $@ $(BUILDDIR)/interpret.o $(BUILDDIR)/common.o

build/common.o: src/common.c src/common.h build
	$(CC) $(CFLAGS) -c $< -o $(BUILDDIR)/common.o

build/test_executable: cube.c bin
	$(CC) $(CFLAGS) $< -o $@

build:
	mkdir -p build

clean:
	if [ -d "build" ]; then rm build/*.o; fi
	find tests -name "*.myout" -delete

test: $(BINARIES) test.sh tests
	./test.sh
