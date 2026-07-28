CC ?= clang
CXX ?= clang++
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -g
AR ?= ar
SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

all: libmsftrace.a tools/msfinspect

libmsftrace.a: $(OBJ)
	$(AR) rcs $@ $^

src/%.o: src/%.c src/internal.h src/msftrace.h src/semantic.h
	$(CC) $(CFLAGS) -Isrc -c $< -o $@

tools/msfinspect: tools/msfinspect.c libmsftrace.a
	$(CC) $(CFLAGS) -Isrc $< libmsftrace.a -o $@

test: tests/test_api tests/test_semantic_utilities
	./tests/test_api
	./tests/test_semantic_utilities

tests/test_api: tests/test_api.c libmsftrace.a
	$(CC) $(CFLAGS) -Isrc $< libmsftrace.a -o $@

tests/test_semantic_utilities: tests/test_semantic_utilities.c libmsftrace.a
	$(CC) $(CFLAGS) -Isrc $< libmsftrace.a -o $@

clean:
	rm -f src/*.o libmsftrace.a tools/msfinspect tests/test_api tests/test_semantic_utilities dev_tasks/bin/*

.PHONY: all test clean
