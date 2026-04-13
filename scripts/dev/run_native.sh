#!/usr/bin/env bash
set -euo pipefail

if [ ! -x build/native/bobtricks ]; then
    echo "No existe build/native/bobtricks. Ejecuta scripts/build/build_native.sh primero."
    exit 1
fi

./build/native/bobtricks
