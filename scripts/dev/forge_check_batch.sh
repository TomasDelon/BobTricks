#!/usr/bin/env bash
set -euo pipefail

BRANCH_NAME="${FORGE_BRANCH_NAME:-forge-publish}"
BASE_REF="${FORGE_BASE_REF:-refs/remotes/forge/main}"
FORBIDDEN_PATTERNS=(
    ".githooks/"
    ".tools/"
    ".agents/"
    "ai/"
    "doc/workflow/"
    "scripts/"
)

if ! git show-ref --verify --quiet "refs/heads/$BRANCH_NAME"; then
    echo "Missing local forge branch '$BRANCH_NAME'."
    echo "Run ./scripts/dev/forge_prepare_batch.sh first."
    exit 1
fi

if ! git show-ref --verify --quiet "$BASE_REF"; then
    echo "Missing base ref '$BASE_REF'."
    echo "Fetch forge/main or complete the first forge bootstrap before running this check."
    exit 1
fi

worktree_dir="$(mktemp -d /tmp/bobtricks-forge-check-XXXXXX)"

cleanup() {
    git worktree remove --force "$worktree_dir" >/dev/null 2>&1 || true
    rm -rf "$worktree_dir"
}
trap cleanup EXIT

git worktree add --detach "$worktree_dir" "$BRANCH_NAME" >/dev/null

changed_files="$(git -C "$worktree_dir" diff --name-only "$BASE_REF"..HEAD)"

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
cmake -S "$worktree_dir" -B "$worktree_dir/build/native"
cmake --build "$worktree_dir/build/native" -j

if [ -d "$worktree_dir/build/native" ]; then
    if ctest --test-dir "$worktree_dir/build/native" -N >/tmp/bobtricks-ctest-list.txt 2>/dev/null; then
        if ! grep -q "Total Tests: 0" /tmp/bobtricks-ctest-list.txt; then
            echo "Running registered native tests..."
            ctest --test-dir "$worktree_dir/build/native" --output-on-failure
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
