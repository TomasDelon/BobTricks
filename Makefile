.PHONY: help build run docs clean

CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic
INCLUDES := -Iinclude $(shell sdl2-config --cflags)
LIBS     := $(shell sdl2-config --libs)

SRCS     := src/main.cpp src/app.cpp
BIN      := build/bobtricks

help:
	@echo "Available targets:"
	@echo "  make help"
	@echo "  make build"
	@echo "  make run"
	@echo "  make clean"
	@echo "  make docs"

build:
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(BIN) $(LIBS)

run:
	@if [ ! -x $(BIN) ]; then echo "Run 'make build' first."; exit 1; fi
	./$(BIN)

clean:
	rm -rf build

docs:
	doxygen doc/documentation/Doxyfile
