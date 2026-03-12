# BobTricks

BobTricks is a 2D procedural stickman locomotion project developed for the **LIFAPCD** course at
Universite Claude Bernard Lyon 1.

The current implementation direction is:

- midpoint demo: `Stand`, `Walk`, `Run`
- midpoint runtime: procedural only
- final target: extend the same architecture toward `Crouch`, `Jump`, `Land`, `Fall`, `Recovery`,
  and `GetUp`

## Project Entry Points

Start here for the current project documentation:

- [Implementation freeze](doc/project/implementation/README.md)
- [Project docs](doc/project/README.md)

## Build Requirements

- `CMake` 3.20 or newer
- `SDL2` for native builds

## Repository Layout

- `src/`: main source code
- `include/`: shared headers when needed
- `doc/`: project documentation
- `data/`: presets, inputs, and generated data that belong to the project

## Common Commands

```bash
make help
make build
make run
```
