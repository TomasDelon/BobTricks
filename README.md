# BobTricks

BobTricks is a 2D procedural stickman locomotion project developed for the **LIFAPCD** course at
Universite Claude Bernard Lyon 1.

The current implementation direction is:

- midpoint demo: `Stand`, `Walk`, `Run`
- midpoint runtime: procedural only
- final target: extend the same architecture toward `Crouch`, `Jump`, `Land`, `Fall`, `Recovery`,
  and `GetUp`

## Project Entry Points

Start here if you want the current source of truth:

- [Implementation freeze](doc/project/implementation/README.md)
- [Project docs](doc/project/README.md)
- [Workflow docs](doc/workflow/README.md)
- [Team Git workflow](doc/workflow/team-git-workflow.md)

## Build Requirements

- `CMake` 3.20 or newer
- `SDL2` for native builds

## Repository Layout

- `src/`: main source code
- `include/`: shared headers when needed
- `doc/`: project and workflow documentation
- `scripts/`: project build and development scripts
- `data/`: presets, inputs, and generated data that belong to the project

## Common Commands

```bash
make help
make build-native
make run-native
```

## Publication Model

This repository uses two remotes with different roles:

- `origin`: full development history on GitHub
- `forge`: curated academic publication history on the university GitLab forge

After the forge bootstrap exists, publication can be automated from local hooks:

- pushes to `origin` may automatically publish eligible non-AI commits to `forge`
- direct pushes to `forge` remain blocked

The exact publication workflow is documented in `doc/workflow/` and will remain explicit.
