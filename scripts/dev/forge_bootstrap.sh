#!/usr/bin/env bash
set -euo pipefail

LAST_PUBLISHED_KEY="bobtricks.forgeLastPublishedSource"

if [ "$#" -gt 1 ]; then
    echo "Usage: $0 [<source-ref>]"
    echo "Bootstrap forge from a clean snapshot of <source-ref> (default: HEAD)."
    exit 1
fi

SOURCE_REF="${1:-HEAD}"

if [ "${ALLOW_DIRTY_BOOTSTRAP:-0}" != "1" ] && { ! git diff --quiet || ! git diff --cached --quiet; }; then
    echo "Working tree is not clean."
    echo "Commit or stash changes before bootstrapping forge."
    exit 1
fi

if [ "${FORGE_FORCE_BOOTSTRAP:-0}" != "1" ] && git show-ref --verify --quiet refs/remotes/forge/main; then
    echo "forge/main already exists."
    echo "Use the regular forge batch workflow instead of bootstrap."
    exit 1
fi

if ! git rev-parse --verify --quiet "$SOURCE_REF^{commit}" >/dev/null; then
    echo "Unknown source ref: $SOURCE_REF"
    exit 1
fi
SOURCE_COMMIT="$(git rev-parse "$SOURCE_REF^{commit}")"

current_branch="$(git branch --show-current)"
source_repo_dir="$(pwd)"
worktree_dir="$(mktemp -d /tmp/bobtricks-forge-bootstrap-XXXXXX)"
bootstrap_branch="forge-bootstrap-$(date +%s)-$$"

cleanup() {
    git worktree remove --force "$worktree_dir" >/dev/null 2>&1 || true
    rm -rf "$worktree_dir"
}
trap cleanup EXIT

git worktree add --detach "$worktree_dir" HEAD >/dev/null

cd "$worktree_dir"
git switch --orphan "$bootstrap_branch" >/dev/null 2>&1

find . -mindepth 1 -maxdepth 1 ! -name .git -exec rm -rf {} +
git rm -rf --cached . >/dev/null 2>&1 || true

git checkout "$SOURCE_COMMIT" -- .
"$source_repo_dir/scripts/dev/derive_forge_snapshot.sh" "$worktree_dir"
git add -A

source_sha="$(git rev-parse --short "$SOURCE_COMMIT")"
git commit -m "Initial project structure"

echo "Running native build validation..."
cmake -S . -B build/native
cmake --build build/native -j

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
if [ "${FORGE_FORCE_BOOTSTRAP:-0}" = "1" ]; then
    ALLOW_FORGE_PUBLISH=1 git push --force forge HEAD:main
else
    ALLOW_FORGE_PUBLISH=1 git push forge HEAD:main
fi

git -C "$source_repo_dir" config --local "$LAST_PUBLISHED_KEY" "$SOURCE_COMMIT"

echo
echo "Forge bootstrap completed."
echo "Original branch remains unchanged: $current_branch"
