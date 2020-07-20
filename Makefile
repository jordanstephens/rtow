SRC = $(wildcard src/*.cc)
OUT = ./build/rtow

test: build
	$(OUT) 960 540 32 > image.ppm && eog image.ppm

.PHONY: build
build:
	g++ -pedantic -Wall -Wextra -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)

default: build
