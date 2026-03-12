.PHONY: help build run docs

help:
	@echo "Available targets:"
	@echo "  make help"
	@echo "  make build"
	@echo "  make run"
	@echo "  make docs"

build:
	cmake -S . -B build/native
	cmake --build build/native -j

run:
	@if [ ! -x build/native/bobtricks ]; then echo "Run 'make build' first."; exit 1; fi
	./build/native/bobtricks

docs:
	doxygen doc/documentation/Doxyfile
