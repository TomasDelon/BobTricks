# Midpoint Procedural Locomotion Spec

## 1. Purpose

This document freezes the **minimum kinematic specification** required to implement a successful
midpoint demo.

It exists because midpoint risk is no longer architectural only.
The project now needs a concrete locomotion recipe for:

- `Stand`
- `Walk`
- `Run`

The goal is not to reach final-quality motion in one week.
The goal is to define motion clearly enough that the team can code against one shared target.

## 2. Source Grounding

This midpoint spec is aligned with the existing dossier:

- `deep-research-report.md` emphasizes CoM-first reasoning, task-oriented reconstruction, and a
  meaningful distinction between walking and running
- `04_Control_Architecture.md` already frames walking as pendular transport and running as
  spring-mass-like behavior
- `15_Validation_Principles.md` explicitly requires that running must not be just "faster walking"

Therefore the midpoint spec adopts:

- walking as a support-alternating pendular transport pattern
- running as a support-alternating spring-mass-like pattern with real flight
- fixed-step procedural timing at `60 Hz`

## 3. Midpoint Control Decision

For midpoint, `GaitPhase` is driven by **pure timer**.

This means:

- phase transitions are triggered by cycle time only
- contact does not trigger phase transitions
- "foot reached target" does not trigger phase transitions
- all team members must use the same timer-driven phase logic

This decision is correct for midpoint because:

- it removes contact and physics dependencies
- it is deterministic and easy to debug
- it prevents incompatible partial implementations

## 4. Shared Driver Variables

The midpoint locomotion controller advances one normalized gait cycle:

```cpp
normalized_cycle = fmod(normalized_cycle + dt / cycle_duration_s, 1.0);
```

The controller must own at least:

- `LocomotionMode mode`
- `GaitPhase gait_phase`
- `double normalized_cycle`
- `double gait_phase_time`
- `double cycle_duration_s`
- `double desired_forward_speed`
- `SupportSide support_side`
- `FootTarget active_swing_target`

`normalized_cycle` is the authoritative phase driver for midpoint.

## 5. Shared Midpoint Rules

These rules apply to both `Walk` and `Run`.

### 5.1 Support Ownership

- the stance foot remains locked to the ground during its support phase
- the swing foot target is computed at swing start
- the swing foot follows a continuous arc toward that target
- no stance foot sliding is allowed in nominal motion

### 5.2 Foot Swing Arc

For midpoint, the swing foot path is:

- horizontal motion: linear interpolation from takeoff position to target position
- vertical motion: one smooth bump, for example `lift * sin(pi * u)`

where:

- `u in [0, 1]` is the normalized swing progress

This is intentionally simple and easy to debug.

### 5.3 Step Target Policy

At the start of each swing phase:

- compute a new nominal target for the swing foot
- base that target on current mode and desired forward speed
- clamp step length to a mode-specific range

Midpoint step target rule:

```text
step_length = desired_forward_speed * cycle_duration_s * 0.5
```

with mode-specific clamp limits.

This gives a simple, deterministic relation between forward speed and gait geometry.

### 5.4 Mode Transitions

Midpoint mode switching must be phase-aware but simple.

Rules:

- `Stand -> Walk` or `Stand -> Run`: allowed immediately, cycle resets to `0`
- `Walk -> Run` and `Run -> Walk`: commit at the next cycle boundary
- no mid-swing mode snap

This prevents visible discontinuities during the demo.

## 6. `Stand`

### 6.1 Goal

`Stand` is a quiet support state.

### 6.2 State

- `GaitPhase::None`
- `SupportSide::Both`
- `desired_forward_speed = 0`

### 6.3 Midpoint Behavior

- both feet remain planted
- pelvis stays near nominal standing height
- optional micro-sway is allowed but not required

`Stand` is not a locomotion cycle.
It is the baseline support posture.

## 7. `Walk`

### 7.1 Behavioral Goal

Walking must read as pendular transport with alternating support.

It must visibly differ from running by:

- having double-support windows
- having no flight
- having smaller vertical oscillation
- having milder trunk and arm contribution

### 7.2 Phase Partition

For midpoint walking, one full gait cycle uses:

- `[0.00, 0.10)` -> `DoubleSupport`
- `[0.10, 0.50)` -> `LeftSupport`
- `[0.50, 0.60)` -> `DoubleSupport`
- `[0.60, 1.00)` -> `RightSupport`

The two double-support windows share the same enum value.
The controller may distinguish early-cycle and late-cycle windows through `normalized_cycle`.

### 7.3 Nominal Starting Parameters

All numbers here are **initial tuning baselines**, not final truth.
They are expressed relative to leg length so that the implementation stays geometry-aware.

- `cycle_duration_s = 1.00`
- `walk_step_length = 0.30 * leg_length`
- `walk_swing_foot_lift = 0.08 * leg_length`
- `walk_pelvis_bob = 0.03 * leg_length`
- `walk_torso_lean = 4 deg`
- `walk_arm_swing = 18 deg`

### 7.4 Pelvis And CM Style

Walking should feel pendular rather than springy.

Midpoint rule:

- forward pelvis transport is monotonic at desired walking speed
- vertical pelvis motion is moderate
- the vertical oscillation should be smaller than the running regime under the same geometry

A simple baseline is:

```text
pelvis_y = base_height - walk_pelvis_bob * cos(4 * pi * normalized_cycle)
```

This creates two vertical extrema per full gait cycle, which is appropriate for walking.

### 7.5 Trunk And Arms

- trunk lean is small and forward
- arm swing is opposite to the corresponding leg swing
- arm swing amplitude scales with walking intensity but remains milder than in running

## 8. `Run`

### 8.1 Behavioral Goal

Running must read as spring-mass-like locomotion with real flight.

It must visibly differ from walking by:

- having no double-support phase
- having explicit flight windows
- having larger vertical motion or stance compression
- having stronger trunk and arm contribution

### 8.2 Phase Partition

For midpoint running, one full gait cycle uses:

- `[0.00, 0.35)` -> `LeftSupport`
- `[0.35, 0.50)` -> `Flight`
- `[0.50, 0.85)` -> `RightSupport`
- `[0.85, 1.00)` -> `Flight`

This is intentionally simple and symmetric enough for a first implementation.

### 8.3 Nominal Starting Parameters

- `cycle_duration_s = 0.70`
- `run_step_length = 0.45 * leg_length`
- `run_swing_foot_lift = 0.12 * leg_length`
- `run_stance_compression = 0.05 * leg_length`
- `run_flight_lift = 0.04 * leg_length`
- `run_torso_lean = 10 deg`
- `run_arm_swing = 28 deg`

### 8.4 Pelvis And CM Style

Running should feel more compliant and spring-like than walking.

Midpoint rule:

- during support, the pelvis compresses downward
- during flight, the pelvis regains height and remains visibly airborne

A simple phase-dependent baseline is:

- support phase: `pelvis_y = base_height - run_stance_compression * sin(pi * u_support)`
- flight phase: `pelvis_y = base_height + run_flight_lift * sin(pi * u_flight)`

This is enough to create a visible difference between walking and running without needing full
physical simulation.

### 8.5 Trunk And Arms

- trunk lean is stronger than in walking
- arm swing is stronger than in walking
- arm timing remains opposite to leg timing

## 9. Pose Reconstruction Expectations

Midpoint pose generation may remain simple as long as the architecture is correct.

Expected reconstruction strategy:

- pelvis anchor from procedural CM target
- feet from stance lock or swing target
- two-segment IK for each leg
- arms driven procedurally from cycle timing
- trunk orientation from mode-specific lean target

The midpoint does not need full-body optimization.
It needs coherent, debuggable, support-aware pose generation.

## 10. Midpoint Acceptance Criteria

The midpoint implementation should be accepted only if:

### 10.1 `Stand`

- both feet stay planted
- posture is stable
- no obvious drift appears

### 10.2 `Walk`

- support alternates left/right with double-support windows
- no flight occurs
- stance foot does not slide
- forward transport remains monotonic
- vertical oscillation stays moderate

### 10.3 `Run`

- a real flight phase exists
- running is visually distinct from walking
- stance compression or vertical oscillation is stronger than walking
- trunk and arms contribute more strongly than in walking

## 11. What This Spec Freezes

This document freezes:

- pure timer as the midpoint gait driver
- explicit phase partitions for walk and run
- the minimum set of body-relative starting parameters
- the distinction between pendular walk and spring-like run
- the requirement that running must contain real flight

This document does **not** freeze final tuning values.
Those may still evolve through debugging and iteration.
