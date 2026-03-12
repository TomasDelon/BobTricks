#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../../.." && pwd)"
FEATURE_DIR="$(cd "$(dirname "$0")/.." && pwd)"

if [ ! -f "$ROOT_DIR/build/web/bobtricks.html" ]; then
    echo "No existe build/web/bobtricks.html. Ejecuta scripts/build/build_web.sh primero."
    exit 1
fi

cd "$FEATURE_DIR"
npm run open:web
