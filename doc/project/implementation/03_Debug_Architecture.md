# Debug Architecture

## 1. Purpose

The debug system is part of the architecture, not a collection of ad hoc helpers.

It must support:

- console interaction
- ImGui interaction
- shared commands
- shared tuning access
- shared state inspection
- node trajectory tracing
- time scaling

## 2. Architectural Rule

There is exactly one debug backend.

There may be multiple debug frontends.

## 3. Backend And Frontends

### 3.1 `DebugCommandBus`

Responsibilities:

- register debug commands
- own command handlers
- expose current debug-readable snapshot
- own tuning registry access
- own trajectory recorder access
- manage pause and time scale
- publish warnings and runtime health messages
- own the incoming debug command queue

Important rule:

- all state-changing debug actions are applied on the main simulation thread
- background producers may enqueue commands, but they do not mutate simulation truth directly

### 3.2 `ConsoleDebugUI`

Responsibilities:

- read textual commands
- display textual state
- use the same backend API as ImGui

Important rule:

- the console UI is not a second debug system
- it is only a second frontend

### 3.3 `ImGuiDebugUI`

Responsibilities:

- render controls and state panels
- call the same backend actions as the console
- display graphs, toggles, and trajectory widgets

## 4. Why Console Debug Matters

Console debug is important for this project because it enables:

- headless test runs
- scripted experimentation
- automation-friendly workflows
- debugging even when graphical UI is broken
- parity with future browser automation tools

The console must not be treated as optional plumbing.
It is a primary debug surface.

## 5. Command Categories

The backend should group commands into clear categories.

### 5.1 Runtime Commands

- `help`
- `status`
- `pause`
- `resume`
- `reset`
- `step <count>`
- `timescale <value>`

### 5.2 Locomotion Commands

- `set_mode stand`
- `set_mode walk`
- `set_mode run`
- later: `crouch`, `jump`, `land`, `fall`, `recovery`, `getup`

### 5.3 Parameter Commands

- `set <group.param> <value>`
- `get <group.param>`
- `list_params`
- `load_preset <name>`
- `save_preset <name>`

Preset storage path:

- `data/presets/<name>.json`

Midpoint note:

- in-memory parameter editing is mandatory
- preset file persistence is useful but may remain minimal in midpoint

### 5.4 Trace Commands

- `trace.node <node_name>`
- `trace.clear`
- `trace.enable`
- `trace.disable`
- `trace.window <seconds>`

### 5.5 Diagnostics Commands

- `dump_state`
- `dump_phase`
- `dump_tuning`
- `dump_trace <node_name>`

## 6. Node Trajectory Tracing

The debug backend must support selecting a node and tracing its trajectory over time.

This is required for:

- feet
- hands
- center-like torso points
- any other key motion anchor

### 6.1 Data Ownership

Trajectory history belongs to the debug backend, not to ImGui and not to the renderer.

The backend records samples from simulation state at fixed-step update time.

### 6.2 Visualization

The same trajectory data is consumed by:

- console output
- ImGui plots
- render overlays

### 6.3 Why Fixed-Step Sampling Matters

Trajectory sampling must happen at fixed simulation steps, not at render frames.
Otherwise the trace changes with frame rate and becomes less useful for debugging.

## 7. Time Scale

Time scale is a backend-controlled runtime feature.

It must be accessible from:

- console
- ImGui

Supported modes:

- paused
- slow motion
- normal time
- accelerated time

The animation system must not implement separate slow-motion logic.
The runtime scales simulation time before fixed-step advancement.

## 8. Console Execution Model

Console input is blocking by nature.
Therefore the architecture must isolate it from the main simulation loop.

This is no longer an open choice.
Midpoint and final both use the following model:

1. `ConsoleDebugUI` owns a dedicated input thread
2. that thread reads whole lines from stdin
3. it parses them into `DebugCommand` objects
4. it pushes them into a thread-safe queue owned by `DebugCommandBus`
5. `SimulationLoop` drains that queue at the start of each fixed simulation step on the main thread

This gives a simple and explicit contract:

- console input may be asynchronous
- ImGui submissions happen on the main thread
- simulation truth is mutated only during main-thread queue draining

The rest of the system only sees parsed commands and main-thread command application.

## 9. ImGui Implementation Note

ImGui is a frontend only.

It may expose:

- current mode and phase
- tuning sliders
- trajectory toggles
- runtime stats
- current time scale
- selected trace node

But it must not own the debug truth.

## 10. Midpoint Minimum Debug Set

The midpoint demo must at least support:

- console `help`
- console `status`
- console `set_mode`
- console `timescale`
- console `trace.node`
- console `trace.clear`
- console `dump_phase`
- ImGui display of current mode
- ImGui display of current gait phase
- ImGui control for time scale
- ImGui node trace visualization

This is enough to support meaningful debugging without overbuilding the UI.
