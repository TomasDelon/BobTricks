# Runtime Architecture

## 1. Runtime Goals

The runtime must:

- run the simulation at fixed 60 Hz
- remain stable under fluctuating rendering speed
- keep rendering outside the core
- keep event polling outside the renderer
- support headless execution
- support optional UI frontends

## 2. Root Object

`main` creates exactly one top-level object:

- `Application`

`Application` is responsible for assembling the runtime graph.

## 3. Runtime Composition

The runtime is decomposed into the following objects.

### 3.1 `Application`

Responsibilities:

- create the runtime graph
- own major subsystems
- choose whether the app runs with SDL, console debug, ImGui, headless mode, or a combination
- call `run()`

### 3.2 `SimulationLoop`

Responsibilities:

- own the fixed-step accumulator
- advance simulation time
- enforce 60 Hz updates
- apply time scaling
- pause/resume
- guard against overload

### 3.3 `PlatformRuntime`

Responsibilities:

- provide wall-clock timing
- poll platform events
- translate low-level input events into abstract application input events

Concrete midpoint implementation:

- `SDLPlatformRuntime`

Important rule:

- event polling belongs here, not in `SDLRenderer`

### 3.4 `InputMapper`

Responsibilities:

- convert platform-level events into `IntentRequest`
- convert keyboard or controller bindings into abstract locomotion commands
- stay independent from the locomotion implementation itself

Important rule:

- raw SDL events must stop here
- the core never receives SDL events directly

### 3.5 `SimulationCore`

Responsibilities:

- own and update the simulation truth
- process locomotion commands
- update locomotion state
- produce renderable state snapshots
- publish debug state

This object must run without SDL and without ImGui.

### 3.6 `DebugCommandBus`

Responsibilities:

- accept debug commands from console and ImGui
- serialize world-changing debug actions onto the main simulation thread
- expose debug-readable snapshots
- manage trajectory traces
- manage time scale and pause requests

### 3.7 `RenderStateAdapter`

Responsibilities:

- transform authoritative simulation state into `RenderState`
- gather visible node positions, debug lines, labels, and trace polylines
- keep rendering code independent from core logic

This keeps the old design intent while avoiding direct renderer access to core internals.

### 3.8 `Renderer`

Responsibilities:

- render `RenderState`
- render debug overlays already expressed as render data
- present frames

Concrete midpoint implementation:

- `SDLRenderer`

Important rule:

- the renderer does not poll input
- the renderer does not own the main loop
- the renderer does not decide locomotion

### 3.9 Debug Frontends

Two frontends exist from the start:

- `ConsoleDebugUI`
- `ImGuiDebugUI`

They depend on the same backend bus.

## 4. Execution Modes

The runtime must support these execution modes.

### 4.1 SDL + ImGui + Console

Normal development mode.

### 4.2 SDL + Console

Graphical mode without ImGui.

### 4.3 Headless + Console

Testing mode without SDL rendering.

### 4.4 Automated Browser Mode

Core plus web build plus external browser automation tooling.
This remains outside the academic architecture but the core must support it naturally.

## 5. Fixed-Step Policy

### 5.1 Simulation Step

- `dt_sim = 1.0 / 60.0`

### 5.2 Real Time Accumulation

Each frame:

1. measure elapsed real time
2. multiply by current time scale
3. add to the simulation accumulator
4. run as many fixed simulation steps as needed
5. render once

### 5.3 Time Scale

The runtime supports:

- pause: `time_scale = 0.0`
- slow motion: `0.0 < time_scale < 1.0`
- normal speed: `time_scale = 1.0`
- fast forward: `time_scale > 1.0`

This is a debug feature and must not be implemented inside animation formulas.

### 5.4 Overload Protection

The runtime must protect itself from the spiral of death.

Policy:

- cap the number of simulation steps per rendered frame
- if overload persists, drop excess accumulated time and emit a debug warning

This ensures the simulation remains usable even on slower machines.

## 6. Runtime Order Per Frame

Each outer frame follows this order:

1. poll platform events
2. let `InputMapper` translate raw input into abstract intent
3. let `ConsoleDebugUI` continue feeding the debug queue from its input thread
4. update accumulator from real time
5. run zero or more fixed simulation steps
6. build `RenderState` through `RenderStateAdapter`
7. render main scene
8. render debug overlays and ImGui
9. present frame

## 7. Runtime Order Per Simulation Step

Each fixed simulation step follows this order:

1. drain pending debug commands on the main thread
2. consume the latest abstract locomotion intent
3. update `LocomotionMode` and timer-driven `GaitPhase`
4. update procedural CM targets and swing-foot targets
5. update node transforms and solved pose state
6. update support-side and phase-dependent state
7. update debug history samplers
8. finalize current authoritative state snapshot

For midpoint, there is no physical branch in this step.

For final, the physical branch is inserted after procedural target generation and before final
state publication.

## 8. Future Physics Integration Point

The final runtime must support a future branch like:

1. procedural target generation
2. physical authority update
3. readback into authoritative character state

This branch exists conceptually from the start, but it is inactive for midpoint.

## 9. Why `SDLRenderer` Must Stay Small

`SDLRenderer` should only know:

- SDL render context
- render primitives
- textures if ever needed later
- how to draw `RenderState`

It should not know:

- how locomotion phases are computed
- how debug commands are parsed
- how time scale is stored
- how input devices map to intent

This keeps rendering replaceable.
