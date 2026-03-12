# Forge Publication Workflow

## Purpose

This document defines the exact workflow for publishing curated academic history to the university
GitLab forge while keeping GitHub as the full development space.

## Chosen Model

The project uses:

- one main development branch on GitHub: `main`
- one academic publication branch on forge: `main`
- one **local-only temporary branch** for publication staging: `forge-publish`

`forge-publish` is not a branch that needs to exist on either remote.
It exists only as a local staging area.

## Why Use A Local `forge-publish` Branch

### Advantages

- it keeps `main` untouched while preparing a forge batch
- it makes publication repeatable and safe
- it allows a batch of `2` to `5` commits to be validated before pushing
- it preserves the individual selected commits instead of forcing a squash
- it isolates forge-only validation from the daily GitHub flow

### Disadvantages

- it adds one extra local branch to manage
- selected commits may conflict when replayed on top of `forge/main`
- the first forge publication needs a clean bootstrap point

For this project, the advantages clearly outweigh the disadvantages.

## Why Cherry-Pick Is Used

Forge publication is based on `git cherry-pick`.

This is the correct mechanism here because:

- it lets the team select only the commits that should appear on `forge`
- it preserves the **Author** of each selected commit by default
- it records a new **Committer** for the person who stages and publishes the batch
- it keeps the individual commit messages and commit boundaries

This means a batch can be pushed together while still showing several separate commits on forge.

## Forbidden Content On Forge

The following paths must never appear in forge publication batches:

- `.tools/`
- `.agents/`
- `ai/`

Any commit that depends on those paths must stay on GitHub only.

## Shared Files Rule

Shared public files such as:

- `README.md`
- `main.cpp`
- public documentation

should remain neutral enough to be publishable to both remotes when possible.

AI-specific instructions and tooling usage must live in private or development-only paths, not in
the shared public entry points.

The main source file is a special case:

- GitHub keeps the development `src/main.cpp`
- forge may receive a derived `src/main.cpp` with the `__EMSCRIPTEN__` branch removed

## Validation Gate Before Publication

Before a batch may be published to forge, it must pass:

- native build success
- all currently registered native tests
- forbidden-path check

`valgrind` becomes mandatory once the project has a stable enough executable and a repeatable memory
check procedure.

## Batch Size

Each forge publication batch should contain:

- minimum: `2` commits
- maximum: `5` commits

This keeps forge history active and readable without flooding it with every intermediate GitHub
commit.

## Exact Routine

### Step 1

Work normally on `origin/main`.

### Step 2

After a successful local push to `origin`, the local `post-push` hook may run automatic forge
publication.

### Step 3

The automation:

- inspects commits on `main` that are not yet mirrored on forge
- filters out commits that touch forbidden paths
- groups eligible commits into batches of `2` to `5`
- replays the batch onto `forge-publish`
- derives an academic `src/main.cpp` from the development version when needed
- validates the batch
- pushes it to `forge/main`

### Step 4

If automation cannot proceed, use the manual staging commands:

```bash
./scripts/dev/forge_prepare_batch.sh <oldest-commit> <next-commit> ...
```

### Step 5

Review and validate the prepared batch:

```bash
./scripts/dev/forge_check_batch.sh
```

### Step 6

If everything is correct, publish the prepared batch:

```bash
./scripts/dev/forge_publish_batch.sh
```

## Safety Mechanism

Direct `git push forge ...` is blocked locally by a Git hook.

The user must go through the publication scripts instead.
This reduces accidental publication of GitHub-only material.

## Author And Committer Behavior

By default, cherry-pick preserves the original commit author.

The person who prepares and pushes the forge batch becomes the committer of the replayed commit.

This is acceptable because it preserves real authorship while allowing delegated publication.

## First Forge Publication

The first forge batch should start from the first clean academic base of the project.

After that initial bootstrap, routine publication should always stage new batches on top of
`forge/main`.

This means:

- bootstrap may still require manual staging
- routine publication after bootstrap may run automatically
