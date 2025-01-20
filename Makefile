.PHONY: all build run test

all: build run

build:
	gcc -std=gnu11 src/*.c \
		-o build/apc \
		-Wall -Wextra -Wpedantic \
		-I src \
		-lm

run:
	./build/apc

test:
	gcc -std=gnu11 test/test.c src/bignum.c \
		-o build/test \
		-Wall -Wextra -Wpedantic \
		-lm
	./build/test
