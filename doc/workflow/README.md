# Workflow Docs

These documents define how work is organized in this repository.

## Files

- `cmake.md`: CMake notes and conventions
- `sdl2.md`: SDL2 notes
- `debug.md`: debugging approach
- `coding_rules.md`: general coding constraints
- `git.md`: generic Git reference
- `team-git-workflow.md`: project-specific teammate workflow for clone, commit, push, and forge automation
- `commits.md`: active commit conventions for this repository
- `remotes-and-publication.md`: GitHub versus GitLab forge publication policy
- `forge-publication-workflow.md`: exact staged publication workflow for forge
- `install.md`: installation notes

## Practical Rules

- `CMake` is the primary build system
- the root `Makefile` is only a wrapper for common commands
- generated artifacts from `build/`, `node_modules/`, and temporary captures are not committed
- `origin` and `forge` do not have the same role: GitHub is full development, forge is curated
  academic publication
- forge publication may be automated locally after pushes to `origin`
