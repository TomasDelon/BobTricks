#!/usr/bin/env bash
set -euo pipefail

BRANCH_NAME="${FORGE_BRANCH_NAME:-forge-publish}"
BASE_REF="${FORGE_BASE_REF:-refs/remotes/forge/main}"
LAST_PUBLISHED_KEY="bobtricks.forgeLastPublishedSource"
PREPARED_BATCH_KEY="bobtricks.forgePreparedBatch"

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
worktree_dir="$(mktemp -d /tmp/bobtricks-forge-worktree-XXXXXX)"

cleanup() {
    rm -rf "$helper_dir"
    git worktree remove --force "$worktree_dir" >/dev/null 2>&1 || true
    rm -rf "$worktree_dir"
}
trap cleanup EXIT

cp "$root_dir/scripts/dev/derive_forge_snapshot.sh" "$helper_dir/derive_forge_snapshot.sh"
cp "$root_dir/scripts/dev/derive_forge_main.sh" "$helper_dir/derive_forge_main.sh"
chmod +x "$helper_dir/derive_forge_snapshot.sh" "$helper_dir/derive_forge_main.sh"

git worktree add --detach "$worktree_dir" "$BASE_REF" >/dev/null
cd "$worktree_dir"
if git show-ref --verify --quiet "refs/heads/$BRANCH_NAME"; then
    git switch "$BRANCH_NAME" >/dev/null
    git reset --hard "$BASE_REF" >/dev/null
else
    git switch -c "$BRANCH_NAME" "$BASE_REF" >/dev/null
fi

for commit in "$@"; do
    source_commit="$(git -C "$root_dir" rev-parse "$commit^{commit}")"
    source_subject="$(git -C "$root_dir" show -s --format=%s "$source_commit")"
    source_body_file="$helper_dir/commit-message.txt"

    find . -mindepth 1 -maxdepth 1 ! -name .git -exec rm -rf {} +
    git rm -rf --cached . >/dev/null 2>&1 || true
    git checkout "$source_commit" -- .

    "$helper_dir/derive_forge_snapshot.sh" "$worktree_dir"
    git add -A

    if git diff --cached --quiet; then
        continue
    fi

    git -C "$root_dir" show -s --format=%B "$source_commit" > "$source_body_file"
    GIT_AUTHOR_NAME="$(git -C "$root_dir" show -s --format=%an "$source_commit")" \
    GIT_AUTHOR_EMAIL="$(git -C "$root_dir" show -s --format=%ae "$source_commit")" \
    GIT_AUTHOR_DATE="$(git -C "$root_dir" show -s --format=%aI "$source_commit")" \
        git commit -F "$source_body_file" >/dev/null

    git config --local "$PREPARED_BATCH_KEY" "$*"
done

git -C "$root_dir" config --local "$PREPARED_BATCH_KEY" "$*"

git -C "$root_dir" ./scripts/dev/forge_check_batch.sh

echo
echo "Forge batch prepared on branch '$BRANCH_NAME'."
echo "Previous branch: $current_branch"
echo "If the batch looks correct, publish it with:"
echo "  ./scripts/dev/forge_publish_batch.sh"
