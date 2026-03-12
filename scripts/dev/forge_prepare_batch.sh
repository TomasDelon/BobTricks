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
root_dir="$(pwd)"
helper_dir="$(mktemp -d /tmp/bobtricks-forge-prepare-XXXXXX)"

cleanup() {
    rm -rf "$helper_dir"
}
trap cleanup EXIT

cp "$root_dir/scripts/dev/derive_forge_snapshot.sh" "$helper_dir/derive_forge_snapshot.sh"
cp "$root_dir/scripts/dev/derive_forge_main.sh" "$helper_dir/derive_forge_main.sh"
chmod +x "$helper_dir/derive_forge_snapshot.sh" "$helper_dir/derive_forge_main.sh"

git switch -C "$BRANCH_NAME" "$BASE_REF"

for commit in "$@"; do
    git cherry-pick -x "$commit"

    "$helper_dir/derive_forge_snapshot.sh" "$root_dir"
    if ! git diff --quiet; then
        git add -A
        git commit --amend --no-edit
    fi
done

./scripts/dev/forge_check_batch.sh

echo
echo "Forge batch prepared on branch '$BRANCH_NAME'."
echo "Previous branch: $current_branch"
echo "If the batch looks correct, publish it with:"
echo "  ./scripts/dev/forge_publish_batch.sh"
