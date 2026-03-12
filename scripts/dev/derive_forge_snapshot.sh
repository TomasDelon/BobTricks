#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="${1:-.}"

remove_path() {
    local target="$1"

    rm -rf "$ROOT_DIR/$target"
}

"$SCRIPT_DIR/derive_forge_main.sh" "$ROOT_DIR/src/main.cpp"

remove_path ".agents"
remove_path ".githooks"
remove_path ".tools"
remove_path "ai"
remove_path "doc/workflow"
remove_path "doc/documentation/output"
remove_path "scripts"
remove_path "src/platform/web"

cat > "$ROOT_DIR/Makefile" <<'EOF'
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
EOF

cat > "$ROOT_DIR/.gitignore" <<'EOF'
# Native build outputs
/build/
/dist/
/bin/*
/obj/*
!.gitkeep

# Generated documentation
/doc/documentation/output/

# Common compiled artifacts
*.o
*.obj
*.out
*.log

# OS/editor
.DS_Store
Thumbs.db
*.swp
EOF

cat > "$ROOT_DIR/README.md" <<'EOF'
# BobTricks

BobTricks is a 2D procedural stickman locomotion project developed for the **LIFAPCD** course at
Universite Claude Bernard Lyon 1.

The current implementation direction is:

- midpoint demo: `Stand`, `Walk`, `Run`
- midpoint runtime: procedural only
- final target: extend the same architecture toward `Crouch`, `Jump`, `Land`, `Fall`, `Recovery`,
  and `GetUp`

## Project Entry Points

Start here for the current project documentation:

- [Implementation freeze](doc/project/implementation/README.md)
- [Project docs](doc/project/README.md)

## Build Requirements

- `CMake` 3.20 or newer
- `SDL2` for native builds

## Repository Layout

- `src/`: main source code
- `include/`: shared headers when needed
- `doc/`: project documentation
- `data/`: presets, inputs, and generated data that belong to the project

## Common Commands

```bash
make help
make build
make run
```
EOF
