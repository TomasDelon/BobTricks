#!/usr/bin/env bash
set -euo pipefail

SOURCE_FILE="${1:-src/main.cpp}"

if [ ! -f "$SOURCE_FILE" ]; then
    echo "Missing source file: $SOURCE_FILE"
    exit 1
fi

tmp_file="$(mktemp)"
trap 'rm -f "$tmp_file"' EXIT

awk '
BEGIN {
    depth = 0
    active[0] = 1
}

/^[[:space:]]*#ifdef[[:space:]]+__EMSCRIPTEN__[[:space:]]*$/ {
    depth++
    parent_active[depth] = active[depth - 1]
    condition_true[depth] = 0
    active[depth] = parent_active[depth] && condition_true[depth]
    next
}

/^[[:space:]]*#ifndef[[:space:]]+__EMSCRIPTEN__[[:space:]]*$/ {
    depth++
    parent_active[depth] = active[depth - 1]
    condition_true[depth] = 1
    active[depth] = parent_active[depth] && condition_true[depth]
    next
}

/^[[:space:]]*#else[[:space:]]*$/ {
    if (depth > 0) {
        active[depth] = parent_active[depth] && !condition_true[depth]
    }
    next
}

/^[[:space:]]*#endif[[:space:]]*$/ {
    if (depth > 0) {
        delete active[depth]
        delete parent_active[depth]
        delete condition_true[depth]
        depth--
    }
    next
}

{
    if (active[depth]) {
        print $0
    }
}
' "$SOURCE_FILE" > "$tmp_file"

if ! cmp -s "$tmp_file" "$SOURCE_FILE"; then
    mv "$tmp_file" "$SOURCE_FILE"
fi
