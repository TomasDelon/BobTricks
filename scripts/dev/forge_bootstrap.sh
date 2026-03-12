#!/usr/bin/env bash
set -euo pipefail

MIN_BATCH_SIZE=1
MAX_BATCH_SIZE=5
FORBIDDEN_PATTERNS=(
    ".tools/"
    ".agents/"
    "ai/"
)

if [ "$#" -lt "$MIN_BATCH_SIZE" ] || [ "$#" -gt "$MAX_BATCH_SIZE" ]; then
    echo "Usage: $0 <oldest-commit> [<next-commit> ...]"
    echo "Select between 1 and 5 commits, ordered from oldest to newest."
    exit 1
fi

if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "Working tree is not clean."
    echo "Commit or stash changes before bootstrapping forge."
    exit 1
fi

if git show-ref --verify --quiet refs/remotes/forge/main; then
    echo "forge/main already exists."
    echo "Use the regular forge batch workflow instead of bootstrap."
    exit 1
fi

for commit in "$@"; do
    while IFS= read -r path; do
        for pattern in "${FORBIDDEN_PATTERNS[@]}"; do
            case "$path" in
                "$pattern"*)
                    echo "Commit $commit touches forbidden forge path: $path"
                    exit 1
                    ;;
            esac
        done
    done < <(git diff-tree --no-commit-id --name-only -r "$commit")
done

current_branch="$(git branch --show-current)"
worktree_dir="$(mktemp -d /tmp/bobtricks-forge-bootstrap-XXXXXX)"

cleanup() {
    git worktree remove --force "$worktree_dir" >/dev/null 2>&1 || true
    rm -rf "$worktree_dir"
}
trap cleanup EXIT

git worktree add --detach "$worktree_dir" HEAD >/dev/null

cd "$worktree_dir"
git switch --orphan forge-publish >/dev/null 2>&1

find . -mindepth 1 -maxdepth 1 ! -name .git -exec rm -rf {} +
git rm -rf --cached . >/dev/null 2>&1 || true

for commit in "$@"; do
    git cherry-pick -x "$commit"

    ./scripts/dev/derive_forge_main.sh src/main.cpp
    if ! git diff --quiet -- src/main.cpp; then
        git add src/main.cpp
        git commit --amend --no-edit
    fi
done

echo "Running native build validation..."
./scripts/build/build_native.sh

if ctest --test-dir build/native -N >/tmp/bobtricks-bootstrap-ctest-list.txt 2>/dev/null; then
    if ! grep -q "Total Tests: 0" /tmp/bobtricks-bootstrap-ctest-list.txt; then
        echo "Running registered native tests..."
        ctest --test-dir build/native --output-on-failure
    else
        echo "No registered native tests found. Skipping test execution."
    fi
else
    echo "CTest metadata not available. Skipping registered test execution."
fi

echo "Publishing forge bootstrap batch to forge/main..."
ALLOW_FORGE_PUBLISH=1 git push forge HEAD:main

echo
echo "Forge bootstrap completed."
echo "Original branch remains unchanged: $current_branch"
