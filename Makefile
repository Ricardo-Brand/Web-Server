.PHONY: all exec clean format build create-dir 

all: format clean build exec

create-dir: ./build
	mkdir build

build: create-dir
	gcc src/main.c -o build/main

exec:
	./build/main

clean:
	rm -rf ./build

format: 
	clang-format -i ./src/*.c