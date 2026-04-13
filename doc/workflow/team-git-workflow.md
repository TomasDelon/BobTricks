# Team Git Workflow

## Purpose

This document explains the day-to-day Git workflow that every project member must follow.

It is project-specific.
It is not a generic Git tutorial.

## One-Time Setup After Cloning

Each team member must do this once after cloning the repository:

```bash
git config user.name "Your Name"
git config user.email "your_email@example.com"
make install-hooks
```

The hook installation is important because:

- direct pushes to `forge` are blocked locally
- pushes to `origin` can trigger automatic forge publication of eligible commits

## Main Rule

Everyone works on:

- `origin/main`

There are no feature branches in the normal workflow.

## Daily Work Routine

### Step 1

Before starting work:

```bash
git switch main
git pull --rebase origin main
```

This keeps your local branch up to date and reduces unnecessary merge commits.

### Step 2

Make your changes locally.

### Step 3

Check what changed:

```bash
git status
git diff
```

### Step 4

Create a small, clean commit:

```bash
git add <files>
git commit -m "Your clear commit message"
```

### Step 5

Before pushing, update again if needed:

```bash
git pull --rebase origin main
```

If there is a conflict, solve it locally before pushing.

### Step 6

Push to GitHub:

```bash
git push origin main
```

After this push:

- GitHub receives the commit normally
- local forge automation may publish eligible commits to `forge`

## Important Commit Rule

Do not mix:

- AI-only changes
- normal project changes

in the same commit.

If a work session touches both, split it into two commits:

1. one commit that only touches `.tools/`, `.agents/`, or `ai/`
2. one commit that only touches normal project files

Why this matters:

- AI-related commits stay on GitHub only
- non-AI commits may be automatically published to forge

## What Happens Automatically

If a pushed commit:

- does not touch `.tools/`, `.agents/`, or `ai/`
- passes the local forge validation rules

then it may be automatically grouped into a small batch and published to `forge/main`.

The publication keeps:

- the original commit author
- the individual commit messages

The automation only changes the committer of the replayed forge commit.

## If Two People Work At The Same Time

This workflow is safe as long as both people do not keep working for too long on an outdated local
`main`.

The rule is:

- pull with rebase before starting
- pull with rebase again before pushing

If both people edit different files, this usually works cleanly.

If both people edit the same file, Git may produce a conflict.
That is normal.
The conflict must be resolved locally before pushing.

## If A Conflict Happens

Typical flow:

```bash
git pull --rebase origin main
```

Then:

- edit the conflicted files
- keep the correct final version
- `git add <resolved-files>`
- `git rebase --continue`

After that:

```bash
git push origin main
```

## What Not To Do

- do not push directly to `forge`
- do not disable the hooks unless there is a clear technical reason
- do not create giant mixed commits
- do not leave local uncommitted work for too long before rebasing

## If Forge Automation Does Not Publish

That usually means one of these:

- the commit touched forbidden paths
- there are not yet enough eligible commits for a batch
- the first forge bootstrap has not happened yet
- validation failed

In that case, the work is still safe on GitHub.
Forge publication can be handled later.
