#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "$ROOT_DIR"

if ! command -v emcmake >/dev/null 2>&1; then
    echo "emcmake was not found. Activate Emscripten before building the web target."
    exit 1
fi

export EM_CACHE="${EM_CACHE:-$ROOT_DIR/build/emscripten-cache}"
mkdir -p "$EM_CACHE"

EM_CONFIG_FILE="$ROOT_DIR/build/emscripten-config.py"
cat > "$EM_CONFIG_FILE" <<EOF
EMSCRIPTEN_ROOT = '/usr/share/emscripten'
LLVM_ROOT = '/usr/bin'
BINARYEN_ROOT = '/usr'
NODE_JS = '/usr/bin/node'
JAVA = 'java'
FROZEN_CACHE = False
CACHE = r'$EM_CACHE'
PORTS = r'$EM_CACHE/ports'
CLOSURE_COMPILER = 'closure-compiler'
LLVM_ADD_VERSION = '15'
CLANG_ADD_VERSION = '15'
EOF
export EM_CONFIG="$EM_CONFIG_FILE"

emcmake cmake -S . -B build/web
cmake --build build/web -j

echo "Web build completed in build/web/"
