#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

fail() {
    printf 'ARCHITECTURE CHECK FAILED: %s\n' "$1" >&2
    exit 1
}

if ! command -v rg >/dev/null 2>&1; then
    fail "rg is required"
fi

if rg -n '#include <SDL|#include "SDL|#include <imgui|#include "imgui' src/core >/dev/null; then
    fail "src/core must stay SDL-free and ImGui-free"
fi

if git ls-files --error-unmatch imgui.ini >/dev/null 2>&1; then
    fail "imgui.ini must remain untracked"
fi

printf 'ARCHITECTURE CHECK OK\n'
