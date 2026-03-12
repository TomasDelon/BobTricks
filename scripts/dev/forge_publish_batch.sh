#!/usr/bin/env bash
set -euo pipefail

BRANCH_NAME="${FORGE_BRANCH_NAME:-forge-publish}"

if [ "$(git branch --show-current)" != "$BRANCH_NAME" ]; then
    echo "Current branch is not '$BRANCH_NAME'."
    echo "Run ./scripts/dev/forge_prepare_batch.sh first."
    exit 1
fi

./scripts/dev/forge_check_batch.sh

echo "Publishing current forge batch to forge/main..."
ALLOW_FORGE_PUBLISH=1 git push forge HEAD:main
