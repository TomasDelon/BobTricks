#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"

if [ ! -f "$ROOT_DIR/build/web/bobtricks.html" ]; then
    echo "Missing build/web/bobtricks.html."
    echo "Run ./.tools/ai-browser-debug/scripts/build_web.sh first."
    exit 1
fi

cd "$ROOT_DIR/build/web"
python3 -m http.server 8080
