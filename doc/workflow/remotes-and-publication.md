# Remotes And Publication Policy

This project uses two Git remotes with different roles.

## Remotes

- `origin`: GitHub
- `forge`: university GitLab forge

## Main Rule

GitHub is the full development space.

GitLab forge only receives the academic publication history that the team wants to present to the
professor as reviewable progress.

The exact staged publication process is defined in:

- `forge-publication-workflow.md`

## What May Exist On GitHub

- AI-related tooling
- agents and skills
- browser-debug infrastructure
- experimental scripts
- exploratory or noisy development commits
- intermediate work states

## What Must Not Go To GitLab Forge

- AI-specific files
- agents
- AI tools and browser-debug infrastructure
- AI-only experimental scripts
- noisy or excessively exploratory commits
- temporary development states
- paths under `.tools/`
- paths under `.agents/`
- paths under `ai/`

## What May Go To GitLab Forge

- project code that belongs to the academic version
- useful and presentable documentation
- small, clear, reviewable commits
- stable progress that the professor can inspect

## Publication Criterion For Forge

A change may be published to `forge` only if it satisfies all of the following:

- it is understandable on its own
- it leaves the project in a reasonably clean state
- it represents real progress on the academic project
- it does not rely on private AI tooling in order to be understood

## Recommended Flow

1. Work normally with `origin` as the main development remote.
2. Use AI tooling and experimentation freely on GitHub.
3. Select the changes that deserve to become part of the academic history.
4. Let the local forge publication automation stage eligible batches when possible.
5. Use manual staging only when automation cannot decide or when bootstrap is still missing.
6. Publish only those presentable changes to `forge`.

## Safety Rule

Do not push to `forge` by accident.

Before any publication to `forge`, review:

```bash
git remote -v
git log --oneline
git diff --stat
```

## Intent

AI is treated as a development tool for the project authors.

GitLab forge is treated as an academic supervision channel, not as a full dump of the private
development environment.
