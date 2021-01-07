all: src/*.cpp
	clang++ src/*.cpp -std=c++2a -O3 -flto -lSDL2 -o chip8
debug: src/*.cpp
	clang++ src/*.cpp -std=c++2a -g -lSDL2 -Wall -o debug
.PHONY: clean
clean:
	rm -rf ./debug.DSYM
	rm -f debug
	rm -f chip8
