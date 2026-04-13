#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../../.." && pwd)"

if [ ! -f "$ROOT_DIR/build/web/bobtricks.html" ]; then
    echo "No existe build/web/bobtricks.html. Ejecuta scripts/build/build_web.sh primero."
    exit 1
fi

SCENARIO="${1:-.tools/ai-browser-debug/sequences/default.json}"
node "$ROOT_DIR/.tools/ai-browser-debug/scripts/run_web_sequence.js" headless "$SCENARIO"
