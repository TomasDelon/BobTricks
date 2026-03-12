#!/usr/bin/env bash
set -euo pipefail

BRANCH_NAME="${FORGE_BRANCH_NAME:-forge-publish}"
BASE_REF="${FORGE_BASE_REF:-refs/remotes/forge/main}"

if [ "$#" -lt 1 ] || [ "$#" -gt 5 ]; then
    echo "Usage: $0 <oldest-commit> [<next-commit> ...]"
    echo "Select between 1 and 5 commits, ordered from oldest to newest."
    exit 1
fi

if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "Working tree is not clean."
    echo "Commit or stash changes before preparing a forge batch."
    exit 1
fi

if ! git show-ref --verify --quiet "$BASE_REF"; then
    echo "Missing base ref '$BASE_REF'."
    echo "Fetch forge/main first, or bootstrap the first forge batch manually."
    exit 1
fi

current_branch="$(git branch --show-current)"

git switch -C "$BRANCH_NAME" "$BASE_REF"
git cherry-pick -x "$@"

./scripts/dev/forge_check_batch.sh

echo
echo "Forge batch prepared on branch '$BRANCH_NAME'."
echo "Previous branch: $current_branch"
echo "If the batch looks correct, publish it with:"
echo "  ./scripts/dev/forge_publish_batch.sh"
