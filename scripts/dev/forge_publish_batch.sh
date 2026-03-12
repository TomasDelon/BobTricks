#!/usr/bin/env bash
set -euo pipefail

BRANCH_NAME="${FORGE_BRANCH_NAME:-forge-publish}"
PREPARED_BATCH_KEY="bobtricks.forgePreparedBatch"
LAST_PUBLISHED_KEY="bobtricks.forgeLastPublishedSource"

if ! git show-ref --verify --quiet "refs/heads/$BRANCH_NAME"; then
    echo "Missing local forge branch '$BRANCH_NAME'."
    echo "Run ./scripts/dev/forge_prepare_batch.sh first."
    exit 1
fi

./scripts/dev/forge_check_batch.sh

echo "Publishing current forge batch to forge/main..."
ALLOW_FORGE_PUBLISH=1 git push forge "refs/heads/$BRANCH_NAME:main"

prepared_batch="$(git config --local --get "$PREPARED_BATCH_KEY" || true)"
if [ -n "$prepared_batch" ]; then
    last_source_commit=""
    for source_commit in $prepared_batch; do
        last_source_commit="$source_commit"
    done

    if [ -n "$last_source_commit" ]; then
        git config --local "$LAST_PUBLISHED_KEY" "$(git rev-parse "$last_source_commit^{commit}")"
    fi
    git config --local --unset "$PREPARED_BATCH_KEY" || true
fi
