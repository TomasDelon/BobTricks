# Source Tree And Ownership

## 1. Purpose

This document defines the implementation-facing source tree.

The objective is to ensure that the coding phase starts with a stable module map rather than
growing a random file layout.

## 2. Top-Level Rule

The source tree must separate:

- application runtime
- platform integration
- input
- core simulation
- rendering
- debug backend
- debug frontends

## 3. Recommended `src/` Layout

```text
src/
├── app/
│   ├── Application.hpp
│   ├── Application.cpp
│   ├── SimulationLoop.hpp
│   └── SimulationLoop.cpp
├── platform/
│   ├── PlatformRuntime.hpp
│   └── sdl/
│       ├── SDLPlatformRuntime.hpp
│       └── SDLPlatformRuntime.cpp
├── input/
│   ├── IntentRequest.hpp
│   ├── InputMapper.hpp
│   └── InputMapper.cpp
├── core/
│   ├── math/
│   │   └── Vec2.hpp
│   ├── state/
│   │   ├── LocomotionMode.hpp
│   │   ├── GaitPhase.hpp
│   │   ├── FootSide.hpp
│   │   ├── SupportSide.hpp
│   │   ├── SupportState.hpp
│   │   ├── NodeId.hpp
│   │   ├── ProceduralCMState.hpp
│   │   ├── PhysicalCMState.hpp
│   │   ├── CMState.hpp
│   │   ├── FootTarget.hpp
│   │   ├── ProceduralPoseState.hpp
│   │   ├── CharacterState.hpp
│   │   └── TuningParams.hpp
│   ├── locomotion/
│   │   ├── LocomotionController.hpp
│   │   ├── LocomotionController.cpp
│   │   ├── ProceduralAnimator.hpp
│   │   └── ProceduralAnimator.cpp
│   ├── simulation/
│   │   ├── SimulationCore.hpp
│   │   └── SimulationCore.cpp
│   └── debug/
│       ├── DebugSnapshot.hpp
│       ├── DebugCommand.hpp
│       ├── DebugCommandBus.hpp
│       ├── DebugCommandBus.cpp
│       ├── NodeTrajectorySample.hpp
│       ├── TrajectoryRecorder.hpp
│       └── TrajectoryRecorder.cpp
├── render/
│   ├── RenderState.hpp
│   ├── RenderStateAdapter.hpp
│   ├── RenderStateAdapter.cpp
│   └── sdl/
│       ├── SDLRenderer.hpp
│       └── SDLRenderer.cpp
├── debug_ui/
│   ├── console/
│   │   ├── ConsoleDebugUI.hpp
│   │   └── ConsoleDebugUI.cpp
│   └── imgui/
│       ├── ImGuiDebugUI.hpp
│       └── ImGuiDebugUI.cpp
└── main.cpp
```

## 4. Ownership Rules

### 4.1 `app/`

Owns the composition root and runtime progression.

### 4.2 `platform/`

Owns platform-specific services:

- SDL event polling
- timing
- window integration support

It does not own locomotion state.

### 4.3 `input/`

Owns translation from raw user input to abstract commands.

`InputMapper` is the explicit boundary between platform events and the core.

It does not own simulation truth.

### 4.4 `core/state/`

Owns passive core value types.

Everything here should mostly be `struct` and enums.

### 4.5 `core/locomotion/`

Owns locomotion logic and procedural animation logic.

### 4.6 `core/simulation/`

Owns orchestration of the core state updates.

### 4.7 `core/debug/`

Owns the backend debug services.

This is where:

- command handling
- snapshots
- traces
- tuning registry access

must live.

### 4.8 `render/`

Owns render-facing data and renderers.

It consumes state but does not produce core truth.

### 4.9 `debug_ui/`

Owns frontend-only debug surfaces.

These are replaceable adapters around the same backend.

## 5. Midpoint Activation Plan

For midpoint, not all modules need full functionality.

### 5.1 Mandatory Midpoint Modules

- `Application`
- `SimulationLoop`
- `SDLPlatformRuntime`
- `InputMapper`
- `SimulationCore`
- `LocomotionController`
- `ProceduralAnimator`
- `RenderState`
- `RenderStateAdapter`
- `SDLRenderer`
- `DebugCommandBus`

### 5.2 Minimal But Real Midpoint Frontends

- `ConsoleDebugUI`
- minimal `ImGuiDebugUI`
- `TrajectoryRecorder`

These may begin with a narrow feature set as long as they already use the correct backend and
ownership model.

### 5.3 Deferred But Predeclared

- physics integration modules
- recovery logic modules
- get-up logic modules
- perturbation modules
- advanced debug panels
- advanced preset persistence

## 6. Public Header Rule

All public types shared across modules should live in named headers, not as hidden anonymous data
inside giant implementation files.

This is especially important for:

- enums
- state structs
- debug command types
- render state types

## 7. Doxygen Rule

All public classes, enums, and nontrivial functions must receive Doxygen comments in French.

Architecture markdown remains in English.

## 8. What Must Not Happen

The following mistakes are explicitly forbidden:

- putting event polling inside `SDLRenderer`
- letting `main` build the app directly from rendering code
- sending SDL events into the core untouched
- making ImGui the owner of debug state
- duplicating one debug implementation for console and another for ImGui
- mixing render geometry and locomotion truth in one state object
