# Design To Implementation Mapping

## 1. Purpose

The project already has a large conceptual design layer.
This document explains how those older design modules map to the implementation-facing classes used
for coding.

Without this mapping, different team members can read the old documents and build incompatible
mental models.

## 2. Mapping Rules

- "Absorbed" means the old responsibility still exists but is implemented inside a larger midpoint
  class
- "Deferred" means the concept remains valid but is not on the midpoint critical path
- "Kept" means the old design name remains the canonical implementation-facing name

## 3. Mapping Table

| Old design module | Midpoint implementation target | Status | Notes |
| --- | --- | --- | --- |
| `SimulationLoop` | `Application` + `SimulationLoop` + `SimulationCore` + `SDLPlatformRuntime` | Split | The old design loop was too large. Timing, platform polling, and simulation ownership are now separated. |
| `InputController` | `InputMapper` + `IntentRequest` | Absorbed | Raw platform input now stops at `InputMapper`. |
| `IntentModule` | `InputMapper` + `LocomotionController` | Absorbed | Intent representation remains explicit through `IntentRequest`. |
| `NominalGaitCycleModule` | `LocomotionController` | Absorbed | Midpoint cycle timing and phase partition live here. |
| `GaitPhaseModule` | `LocomotionController` | Absorbed | `GaitPhase` remains a first-class state type. |
| `RegimeManager` | `LocomotionController` | Absorbed for midpoint | Midpoint only needs nominal procedural mode switching for `Stand`, `Walk`, and `Run`. |
| `CMController` | `LocomotionController` | Absorbed | Procedural CM targets are generated here in midpoint. |
| `FootPlacementPlanner` | `LocomotionController` | Absorbed for nominal midpoint stepping | Only nominal step placement is required in midpoint. Recovery placement is deferred. |
| `ReconstructionIK` | `ProceduralAnimator` | Absorbed | Leg IK, trunk pose shaping, and arm swing generation live here. |
| `RenderStateAdapter` | `RenderStateAdapter` | Kept | This is the canonical view-adapter name. `RenderStateBuilder` is dropped. |
| `ContinuousLineBuilder` | rendering-side helper under `render/` | Deferred or helper-level | Useful for the final look, but not a top-level architecture blocker. |
| `DebugRenderer` | `RenderStateAdapter` + `SDLRenderer` + `ImGuiDebugUI` | Absorbed | Debug visuals are produced as render data, not as separate truth. |
| `SupportAndContactReasoner` | deferred future module | Deferred | Midpoint uses timer-driven gait, not contact-driven support reasoning. |
| `RecoveryMetric` | deferred future module | Deferred | Not on the midpoint critical path. |
| `AuthorityBranchCoordinator` | conceptual responsibility inside `SimulationCore` | Deferred | Needed only once the physical branch is activated. |
| `TransitionSynchronizer` | deferred future module | Deferred | Reserved for procedural/physical transitions and later mode transitions. |
| `GetUpController` | deferred future module | Deferred | Final-scope behavior only. |
| `PerturbationController` | deferred future module | Deferred | Not required for the midpoint demo. |
| `PhysicalBody` | future physics-side blueprint module | Deferred | Not active in midpoint. |
| `Box2DWorldAdapter` | future physics-side runtime module | Deferred | Architecture preserves the integration point but does not activate it now. |
| `ContactCollector` | future physics-side runtime module | Deferred | Not needed before Box2D is active. |
| `PhysicalReadback` | future physics-side runtime module | Deferred | `PhysicalCMState` stays in the state model even while inactive. |

## 4. Midpoint Class Responsibilities

### 4.1 `LocomotionController`

Midpoint ownership:

- mode switching among `Stand`, `Walk`, `Run`
- timer-driven `GaitPhase`
- cycle timing
- nominal step-length and swing-target generation
- procedural CM target generation

### 4.2 `ProceduralAnimator`

Midpoint ownership:

- reconstructing a pose from procedural targets
- maintaining stance foot locking
- generating swing-foot arcs
- applying trunk lean and arm swing

### 4.3 `SimulationCore`

Midpoint ownership:

- authoritative `CharacterState`
- orchestration of `LocomotionController` and `ProceduralAnimator`
- publishing debug snapshots
- publishing state to `RenderStateAdapter`

### 4.4 `SimulationLoop`

Midpoint ownership:

- fixed-step accumulator
- time scaling
- overload protection

It does **not** own locomotion logic directly.

## 5. What This Means For Coding

When coding midpoint:

- do not recreate the old "god object" `SimulationLoop`
- do not create separate midpoint mini-versions of `RegimeManager`, `GaitPhaseModule`, and
  `CMController` unless there is a proven need
- keep the responsibilities, but absorb them into the agreed midpoint classes

This document is the reconciliation layer between the old conceptual dossier and the new coding
freeze.
