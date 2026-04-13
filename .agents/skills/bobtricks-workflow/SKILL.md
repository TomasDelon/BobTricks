---
name: bobtricks-workflow
description: Use this skill when working on the BobTricks repository for C++/SDL2 development, native builds with CMake, web builds with Emscripten, or browser-based validation of the generated web app.
---

# BobTricks Workflow

Use this skill for changes in this repository.

## Project shape

- Native target: `cmake -S . -B build/native && cmake --build build/native -j`
- Native binary: `build/native/bobtricks`
- Web target: `emcmake cmake -S . -B build/web && cmake --build build/web -j`
- Web output: `build/web/bobtricks.html`
- Playwright sequence runner: `scripts/dev/run_web_sequence.js`

## Default workflow

1. Read `CMakeLists.txt` and the relevant files in `src/` and `include/`.
2. Prefer keeping platform differences in small `#ifdef __EMSCRIPTEN__` branches.
3. Do not duplicate native and web code unless the platform API truly requires it.
4. Validate native changes locally after edits.
5. If the task affects browser behavior, build web and serve `build/web/`.
6. For browser tuning tasks, prefer editing a JSON sequence in `scripts/dev/sequences/`.

## Commands

Native build:

```bash
make build-native
```

Native run:

```bash
make run-native
```

Web build:

```bash
make build-web
```

Serve web build:

```bash
make serve-web
```

Headless interaction:

```bash
make interact-web
```

Visible interaction:

```bash
make interact-web-headed
```

## Guardrails

- Do not commit generated files from `build/`.
- Keep source-of-truth code in `src/`, `include/`, and build configuration files.
- Prefer CMake as the primary build system. The `Makefile` is only a thin wrapper.
- If SDL2 behavior differs between native and web, isolate the difference behind a small adapter or conditional compile block.
- For repeatable browser interaction, store scripted steps in JSON instead of hardcoding them into multiple one-off scripts.
