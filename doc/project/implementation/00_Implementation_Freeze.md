# Implementation Freeze

## 1. Purpose

This document freezes the implementation direction of the project so that coding can start without
reopening architectural decisions every day.

The goal is not to define every final numeric tuning value in advance.
The goal is to remove ambiguity about:

- scope
- runtime ownership
- state ownership
- MVC boundaries
- debug architecture
- module decomposition
- midpoint versus final deliverable
- coding priority for the next week

## 2. Midpoint Scope

The midpoint demo must include:

- `Stand`
- `Walk`
- `Run`

The midpoint version is **procedural only**.
It must not depend on Box2D to be considered successful.

This is the correct decision for the available time window because it maximizes:

- implementation focus
- architectural cleanliness
- debugging clarity
- demo reliability

## 3. Final Scope

The final target behavior set is:

- `Stand`
- `Walk`
- `Run`
- `Crouch`
- `Jump`
- `Land`
- `Fall`
- `Recovery`
- `GetUp`

The final architecture must support a future physical authority branch, but the midpoint version
must not be delayed by integrating it too early.

## 4. Main Architectural Decision

The project starts with the **final architecture shape** from day one.
Only the active feature set is reduced.

This means:

- the folder structure already reflects the long-term architecture
- the runtime already separates core, platform, rendering, input, and debug
- the state model already reserves the future physical branch
- the debug system already supports both console and ImGui through one backend

What changes between midpoint and final is not the architecture.
What changes is the set of active modules and behaviors.

## 5. Midpoint Critical Path

For the next week, the project must not try to finish every subsystem equally.

The blocking path is:

1. freeze the midpoint locomotion spec for `Stand`, `Walk`, and `Run`
2. implement timer-driven `GaitPhase`
3. implement `LocomotionController` and `ProceduralAnimator`
4. implement `RenderStateAdapter` and `SDLRenderer`
5. implement minimal console and ImGui debug frontends on top of one backend
6. defer Box2D, recovery, get-up, and advanced physics modules

This order is mandatory because the midpoint can fail even if the architecture is clean but
`Walk` and `Run` are still undefined or visually broken.

## 6. Physics Decision

For midpoint:

- no Box2D in the active path
- no physical authority in the update loop
- no dependency on physics to obtain `Stand`, `Walk`, and `Run`

For final:

- the architecture reserves a clean integration point for Box2D
- physical authority becomes an optional branch of the simulation
- the procedural path remains valid and does not get rewritten

This avoids the common failure mode where early physics integration destroys progress on the basic
locomotion system.

## 7. Root Runtime Decision

`main` must create a top-level `Application` object.

`main` must **not** directly create:

- `SDLRenderer`
- a locomotion controller
- a game loop hidden inside rendering code

The correct ownership is:

- `Application` owns the runtime composition
- `SimulationLoop` owns timing and fixed-step progression
- `PlatformRuntime` owns platform events and wall-clock access
- `Renderer` only renders

This directly resolves the professor's concern about choosing a higher-level object above the SDL
renderer and the simulation loop.

## 8. Core and HMI Separation

The animation core must remain independent from human-machine interfaces.

The core must not know about:

- SDL windows
- ImGui widgets
- console formatting
- keyboard scan codes
- PlayStation controllers

The core only receives abstract input requests and produces abstract state.

This makes it possible to replace the HMI later with:

- keyboard
- gamepad
- scripted tests
- console commands
- browser automation

without rewriting the stickman logic.

## 9. Fixed Time Policy

The simulation uses:

- fixed simulation step: `1 / 60 s`
- render/update decoupling
- time accumulator
- overload protection
- time scaling

The rule is:

- if the machine is fast, the simulation still runs at fixed 60 Hz
- if the machine is slow, the runtime attempts to catch up while protecting itself from the spiral
  of death

This is specified in the runtime document.

## 10. Debug Philosophy

The debug system is a first-class architectural concern.

It is not an afterthought and not a UI trick.

The project must implement:

- one debug backend
- one console frontend
- one ImGui frontend

Both frontends expose the same command set conceptually.
The presentation differs; the capability set does not.

This is required because:

- console debugging must work without graphical UI
- ImGui debugging must remain only a presentation layer
- automated tests and headless runs need the same backend

## 11. Public Coding Policy

All technical code identifiers and architecture documents are written in English.

Doxygen comments are written in French to match the course context.

## 12. Struct vs Class Policy

The codebase follows a strict distinction:

- `struct` for small passive value objects
- `class` for objects with ownership, lifecycle, invariants, or logic

This is mandatory, not stylistic.

Examples that should be `struct`:

- `IntentRequest`
- `CMState`
- `SupportState`
- `FootTarget`
- `TuningParams`
- `NodeTrajectorySample`
- `DebugSnapshot`

Examples that should be `class`:

- `Application`
- `SimulationLoop`
- `ProceduralAnimator`
- `LocomotionController`
- `DebugCommandBus`
- `SDLPlatformRuntime`
- `SDLRenderer`

## 13. Midpoint Work Package Strategy

The midpoint week should be organized around three parallel streams:

- locomotion core: `GaitPhase`, cycle driver, foot targets, procedural pose generation
- runtime and debug: `Application`, `SimulationLoop`, `SDLPlatformRuntime`, `InputMapper`,
  `DebugCommandBus`, minimal console debug
- view and presentation: `RenderStateAdapter`, `SDLRenderer`, continuous-line rendering, minimal
  ImGui integration

Physics, recovery, falling, get-up, and perturbation work are explicitly **outside** the midpoint
critical path.

## 14. Design-To-Implementation Reconciliation

The older design docs remain useful for conceptual intent, but the implementation layer must state
how those modules map to the actual code shape.

The short version is:

- old design `SimulationLoop` is split into `Application`, `SimulationLoop`, `SimulationCore`,
  `PlatformRuntime`, and `RenderStateAdapter`
- midpoint `LocomotionController` absorbs the old nominal intent, gait, regime, and nominal
  foot-placement responsibilities needed for `Stand`, `Walk`, and `Run`
- midpoint `ProceduralAnimator` absorbs the old reconstruction and secondary pose-shaping work
- `RenderStateAdapter` keeps the old design name and remains the bridge from core truth to view data
- recovery, perturbation, physical readback, and Box2D modules remain architecturally reserved but
  inactive for midpoint

The full mapping is documented in `06_Design_To_Implementation_Mapping.md`.

## 15. What Is Still Allowed To Change

The following may still evolve during coding:

- exact numeric tuning values inside the midpoint locomotion spec
- exact naming of some helper methods
- exact contents of tuning parameter groups
- exact visual presentation of debug information

The following should now be considered fixed:

- top-level architecture
- midpoint scope
- final scope
- core/HMI separation
- `GaitPhase` as a dedicated type
- dual `CMState` structure
- debug backend design
- `Application` as root object
- fixed-step runtime
- `RenderStateAdapter` as canonical view-adapter name
- English naming and French Doxygen policy
