#!/usr/bin/env bash
set -euo pipefail

BRANCH_NAME="${FORGE_BRANCH_NAME:-forge-publish}"
BASE_REF="${FORGE_BASE_REF:-refs/remotes/forge/main}"
FORBIDDEN_PATTERNS=(
    ".tools/"
    ".agents/"
    "ai/"
)

if [ "$(git branch --show-current)" != "$BRANCH_NAME" ]; then
    echo "Current branch is not '$BRANCH_NAME'."
    echo "Run ./scripts/dev/forge_prepare_batch.sh first."
    exit 1
fi

if ! git show-ref --verify --quiet "$BASE_REF"; then
    echo "Missing base ref '$BASE_REF'."
    echo "Fetch forge/main or complete the first forge bootstrap before running this check."
    exit 1
fi

changed_files="$(git diff --name-only "$BASE_REF"..HEAD)"

if [ -z "$changed_files" ]; then
    echo "No staged forge batch detected relative to $BASE_REF."
    exit 1
fi

forbidden_hits=()
while IFS= read -r path; do
    for pattern in "${FORBIDDEN_PATTERNS[@]}"; do
        case "$path" in
            "$pattern"* )
                forbidden_hits+=("$path")
                ;;
        esac
    done
done <<< "$changed_files"

if [ "${#forbidden_hits[@]}" -gt 0 ]; then
    echo "Forbidden forge paths detected:"
    printf '  %s\n' "${forbidden_hits[@]}"
    exit 1
fi

echo "Running native build validation..."
./scripts/build/build_native.sh

if [ -d build/native ]; then
    if ctest --test-dir build/native -N >/tmp/bobtricks-ctest-list.txt 2>/dev/null; then
        if ! grep -q "Total Tests: 0" /tmp/bobtricks-ctest-list.txt; then
            echo "Running registered native tests..."
            ctest --test-dir build/native --output-on-failure
        else
            echo "No registered native tests found. Skipping test execution."
        fi
    else
        echo "CTest metadata not available. Skipping registered test execution."
    fi
else
    echo "Missing build/native after build step."
    exit 1
fi

echo "Valgrind is not yet wired into the forge gate. Skipping memory check for now."
echo "Forge batch validation passed."
