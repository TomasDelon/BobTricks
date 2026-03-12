#!/usr/bin/env bash
set -euo pipefail

DEFAULT_BRANCH="${FORGE_SOURCE_BRANCH:-main}"
BRANCH_NAME="${FORGE_BRANCH_NAME:-forge-publish}"
MIN_BATCH_SIZE="${FORGE_MIN_BATCH_SIZE:-2}"
MAX_BATCH_SIZE="${FORGE_MAX_BATCH_SIZE:-5}"

if [ "$(git branch --show-current)" != "$DEFAULT_BRANCH" ]; then
    echo "Forge autopublish skipped: current branch is not '$DEFAULT_BRANCH'."
    exit 0
fi

if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "Forge autopublish skipped: working tree is not clean."
    exit 0
fi

if ! git fetch forge main >/dev/null 2>&1; then
    echo "Forge autopublish skipped: could not fetch forge/main."
    exit 0
fi

if ! git show-ref --verify --quiet refs/remotes/forge/main; then
    echo "Forge autopublish skipped: forge/main does not exist yet."
    echo "Run the first clean forge bootstrap manually."
    exit 0
fi

mapfile -t unpublished_commits < <(
    git cherry forge/main "$DEFAULT_BRANCH" \
    | awk '$1 == "+" { print $2 }' \
    | tac
)

if [ "${#unpublished_commits[@]}" -eq 0 ]; then
    echo "Forge autopublish: no unpublished commits detected."
    exit 0
fi

is_commit_eligible() {
    local commit="$1"
    local path

    while IFS= read -r path; do
        case "$path" in
            .tools/*|.agents/*|ai/*)
                return 1
                ;;
        esac
    done < <(git diff-tree --no-commit-id --name-only -r "$commit")

    return 0
}

publish_batch() {
    local batch=("$@")

    if [ "${#batch[@]}" -lt "$MIN_BATCH_SIZE" ]; then
        return 0
    fi

    echo "Forge autopublish: preparing batch of ${#batch[@]} commit(s)."
    ./scripts/dev/forge_prepare_batch.sh "${batch[@]}"
    ./scripts/dev/forge_publish_batch.sh
    git switch "$DEFAULT_BRANCH" >/dev/null 2>&1 || true
    git fetch forge main >/dev/null 2>&1 || true
}

batch=()
for commit in "${unpublished_commits[@]}"; do
    if is_commit_eligible "$commit"; then
        batch+=("$commit")
        if [ "${#batch[@]}" -ge "$MAX_BATCH_SIZE" ]; then
            publish_batch "${batch[@]}"
            batch=()
        fi
    fi
done

if [ "${#batch[@]}" -ge "$MIN_BATCH_SIZE" ]; then
    publish_batch "${batch[@]}"
elif [ "${#batch[@]}" -eq 1 ]; then
    echo "Forge autopublish: one eligible commit pending; waiting for the next stable commit."
fi

if [ "$(git branch --show-current)" = "$BRANCH_NAME" ]; then
    git switch "$DEFAULT_BRANCH" >/dev/null 2>&1 || true
fi
