#!/usr/bin/env bash
set -euo pipefail

cmake -S . -B build/native
cmake --build build/native -j

echo "Build nativa lista en build/native/"
