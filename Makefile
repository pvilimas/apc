all: build run

build:
	gcc -std=gnu11 src/*.c \
		-o build/apc \
		-Wall -Wextra -Wpedantic \
		-I src \
		-lm

run:
	./build/apc
