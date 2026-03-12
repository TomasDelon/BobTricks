# Commits

These are the active commit conventions for this repository.

## Principles

- a commit must have one clear technical purpose
- do not mix architecture, refactor, and experimental tuning in the same commit if they can be
  separated
- if an experiment is not stable, do not commit it as if it were final
- every commit intended for publication must leave the repository buildable at least in native mode
- not every local or GitHub commit is meant to reach `forge`

## Recommended Message Style

Use short imperative messages, for example:

- `Add gait-phase timer for procedural walk`
- `Refactor runtime ownership around Application`
- `Document midpoint locomotion specification`
- `Tune run stance compression`

## When To Commit

- after closing a small unit of work
- after a verifiable improvement
- before a major change of direction
- before touching a sensitive area that may break several things

## When Not To Commit

- if the change only contains local test garbage
- if generated artifacts are mixed into the change
- if the code only half-builds and there is no clear reason to preserve that state

## Useful Commit Categories

- `docs`: documentation and dossier maintenance
- `build`: CMake, scripts, and toolchain work
- `core`: procedural logic, simulation, and locomotion
- `render`: SDL view, overlays, and visual adapters
- `debug`: debug backend and debug UI work
- `tuning`: justified parameter adjustments

## Recommended Development Flow

1. change code or documentation
2. verify native build
3. review `git diff`
4. create a small, readable commit

## Forge Publication Rule

- `forge` is for stable, presentable commits only
- forge publication happens in small batches of `2` to `5` commits
- the individual commits remain separate in forge history
- the batch is pushed together only after validation succeeds

## Bad Messages

- `fix`
- `changes`
- `more stuff`
- `working version`

## Good Messages

- `Document retained design docs from previous project`
- `Add timer-driven gait phase for walk and run`
- `Refactor SDL runtime to separate event polling`
