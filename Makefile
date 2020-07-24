SRC = $(wildcard src/*.cc)
OUT = ./build/rtow
FLAGS = -fopenmp

test: build
	$(OUT) 960 540 32 > image.ppm && eog image.ppm

.PHONY: build
build:
	g++ -g $(FLAGS) -pedantic -Wall -Wextra -o $(OUT) $(SRC)

build-release:
	g++ $(FLAGS) -O3 -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)

default: build
