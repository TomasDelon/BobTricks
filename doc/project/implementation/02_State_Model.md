# State Model

## 1. Purpose

This document defines the implementation-facing state model.

The goal is to remove ambiguity about:

- core state ownership
- value types versus logic objects
- locomotion mode and phase representation
- render state boundaries
- debug state boundaries

## 2. Core Principle

The simulation core owns the authoritative truth.

The authoritative truth is published through state objects.
Views, input adapters, and debug UIs consume or modify the system only through controlled APIs.

## 3. Fundamental Shared Types

### 3.1 `Vec2`

Midpoint must use a small project-owned vector type:

```cpp
struct Vec2 {
    double x;
    double y;
};
```

Canonical header:

- `src/core/math/Vec2.hpp`

Midpoint must not depend on Eigen just to represent 2D positions and velocities.
That dependency can be introduced later if a clear mathematical need appears.

### 3.2 `LocomotionMode`

This type must exist from the beginning as:

```cpp
enum class LocomotionMode {
    Stand,
    Walk,
    Run,
    Crouch,
    Jump,
    Land,
    Fall,
    Recovery,
    GetUp
};
```

At midpoint only:

- `Stand`
- `Walk`
- `Run`

need active implementations.

The other values still exist now so that the architecture starts in its final shape.

### 3.3 `GaitPhase`

This type must exist from the beginning as a dedicated support-cycle state.

```cpp
enum class GaitPhase {
    None,
    DoubleSupport,
    LeftSupport,
    RightSupport,
    Flight
};
```

This directly answers the professor remark that phase information is missing from the procedural
state.

Important rule:

- `GaitPhase` is not a generic "everything phase" enum
- it only represents the gait-support cycle
- future crouch, jump, land, fall, recovery, and get-up details must not be shoved into it

### 3.4 `FootSide`

```cpp
enum class FootSide {
    Left,
    Right
};
```

### 3.5 `SupportSide`

```cpp
enum class SupportSide {
    None,
    Left,
    Right,
    Both
};
```

### 3.6 `NodeId`

This should be an explicit enum for traceable logical points:

```cpp
enum class NodeId {
    Head,
    TorsoTop,
    TorsoCenter,
    TorsoBottom,
    ElbowLeft,
    WristLeft,
    ElbowRight,
    WristRight,
    KneeLeft,
    AnkleLeft,
    KneeRight,
    AnkleRight
};
```

## 4. Mandatory Core Structs

These should be `struct`, not `class`.

### 4.1 `IntentRequest`

Carries abstract user or scripted intent.

Suggested fields:

- `LocomotionMode requested_mode`
- `bool reset_requested`
- `bool pause_toggle_requested`
- `double requested_time_scale`

### 4.2 `ProceduralCMState`

Suggested fields:

- `Vec2 target_position`
- `Vec2 target_velocity`
- `double operating_height`
- `double pelvis_offset_target`
- `double trunk_lean_target`

This is the nominal CM-side truth used by the procedural locomotion branch.

### 4.3 `PhysicalCMState`

Suggested fields:

- `Vec2 position`
- `Vec2 velocity`
- `Vec2 acceleration`
- `double operating_height_estimate`
- `bool valid`

This is the emergent CM-side truth read back from the future physical branch.

At midpoint it may remain present but invalid.

### 4.4 `CMState`

Suggested fields:

- `ProceduralCMState procedural`
- `PhysicalCMState physical`

This split is required even before Box2D becomes active.
The architecture must never force downstream code to guess whether it is reading a target or an
emergent physical state.

### 4.5 `SupportState`

Suggested fields:

- `SupportSide active_side`
- `bool left_grounded`
- `bool right_grounded`
- `bool support_valid`

### 4.6 `FootTarget`

Suggested fields:

- `FootSide foot`
- `Vec2 takeoff_position`
- `Vec2 target_position`
- `double lift_height`

### 4.7 `ProceduralPoseState`

This must contain the gait phase.

Suggested fields:

- `LocomotionMode mode`
- `GaitPhase gait_phase`
- `double gait_phase_time`
- `double normalized_cycle`
- `double mode_time`
- `SupportSide support_side`
- `double forward_speed`
- `double cycle_duration_s`
- `FootTarget active_swing_target`
- joint or node targets needed by the animator

### 4.8 `CharacterState`

High-level authoritative character snapshot.

Suggested fields:

- `LocomotionMode mode`
- `GaitPhase gait_phase`
- `SupportSide support_side`
- `CMState cm`
- `SupportState support`
- `ProceduralPoseState procedural_pose`
- node positions or solved pose snapshot

### 4.9 `TuningParams`

This holds tweakable procedural parameters.

It is a `struct` because it is mostly passive data.

It should be grouped into sub-structs if it becomes large:

- `LocomotionTuningParams`
- `TimingTuningParams`
- `DebugTuningParams`

### 4.10 `NodeTrajectorySample`

Suggested fields:

- `double sim_time`
- `NodeId node`
- `Vec2 position`

### 4.11 `DebugSnapshot`

Published by the debug backend.

Suggested fields:

- current mode and gait phase
- time scale
- paused state
- selected node
- trace status
- recent warnings
- current tuning values

## 5. Struct vs Class Rule

The rule is explicit:

- a value type with few fields and no ownership is a `struct`
- an object with lifecycle, ownership, invariants, or methods that change the world is a `class`

This is not optional.

## 6. Core Classes

The following are proper classes:

- `SimulationCore`
- `LocomotionController`
- `ProceduralAnimator`
- `SimulationLoop`
- `Application`
- `DebugCommandBus`
- `TrajectoryRecorder`
- `RenderStateAdapter`

## 7. Render State Boundary

The renderer must not consume `CharacterState` directly if that would force it to understand core
logic.

Instead the core or an adapter builds a `RenderState`.

Suggested contents of `RenderState`:

- visible node positions
- curve control points
- debug lines
- trajectory polyline data
- labels to display
- color and thickness values

`RenderState` is a view-facing data product, not simulation truth.

## 8. Input Boundary

The core must not receive SDL events directly.

The core receives:

- `IntentRequest`
- debug commands
- optional scripted events

This is what keeps keyboard, console, browser, and gamepad integration replaceable.

## 9. Midpoint State Activation

At midpoint the active state subset is:

- `LocomotionMode::Stand`
- `LocomotionMode::Walk`
- `LocomotionMode::Run`
- `GaitPhase::None`
- `GaitPhase::DoubleSupport`
- `GaitPhase::LeftSupport`
- `GaitPhase::RightSupport`
- `GaitPhase::Flight`

Specific midpoint use is:

- `Stand` -> `GaitPhase::None`
- `Walk` -> `DoubleSupport`, `LeftSupport`, `RightSupport`
- `Run` -> `LeftSupport`, `Flight`, `RightSupport`

## 10. Final State Growth

Later additions should extend existing structs and enums rather than forcing a rewrite of the
runtime skeleton.

Important rule:

- future modes may introduce dedicated mode-specific substate
- they must not overload `GaitPhase` with unrelated semantics
