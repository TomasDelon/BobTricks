# Modules and Responsibilities

## Implementation Alignment Note

This document remains valid as a **conceptual module dossier**.

It must not be read as the direct coding skeleton for midpoint.
For actual class ownership and folder structure, the following documents take precedence:

- [`../implementation/04_Source_Tree_And_Ownership.md`](../implementation/04_Source_Tree_And_Ownership.md)
- [`../implementation/06_Design_To_Implementation_Mapping.md`](../implementation/06_Design_To_Implementation_Mapping.md)

Important midpoint interpretation:

- several conceptual modules are absorbed into `LocomotionController` and `ProceduralAnimator`
- midpoint keeps dual `CMState` (`procedural` and `physical`)
- midpoint keeps `GaitPhase` as a dedicated type
- recovery, perturbation, and Box2D runtime modules remain deferred for midpoint

---

## 1. Role of This Document

This document defines the **main software modules** of the project and the responsibility of each one.

The previous documents already defined:

* the project vision
* the stickman model
* the CM-centered locomotion logic
* the hybrid regimes
* the control architecture
* the rendering model
* the MVC decomposition
* the physical body model
* the reconstruction logic
* the recovery metric
* the update loop and timing

What is still missing is a module-level decomposition that is concrete enough to prepare:

* the future class diagram
* the future `cahier des charges`
* the implementation TODO list

This document therefore answers:

* which modules should exist
* what each module is responsible for
* what information each module consumes
* what information each module produces
* which dependencies are acceptable and which are not

This document still does **not** define:

* exact class names
* exact method signatures
* final folder structure

Those belong to the later class-design stage.

---

## 2. Why a Module Decomposition Is Needed

The current dossier already defines the system conceptually.
What it still needs is a module structure that prevents the future implementation from collapsing into:

* one oversized `Character` object
* one oversized `Game` loop
* direct coupling between SDL, Box2D, locomotion, and rendering

The project is now detailed enough that this decomposition should no longer remain implicit.

In particular, the project needs to preserve these separations:

* procedural logic vs physical simulation
* support reasoning vs articulated reconstruction
* simulation state vs rendering state
* Model vs View vs Controller

This document is therefore the bridge between conceptual architecture and concrete software design.

---

## 3. Module Decomposition Strategy

For V1, the module structure should be guided by three principles:

### 3.1 One Module per Real Responsibility

A module should exist only if it owns a real responsibility, not just because the code “might become large”.

### 3.2 State and Processing Should Be Separated Where Useful

Some modules mainly own state.
Others mainly compute over existing state.

This distinction is useful in the project because:

* the stickman has persistent state
* the control layers transform that state
* the View only renders it

### 3.3 Dependencies Must Follow the Architecture

Modules should depend in the direction already defined by the dossier:

* intent -> regime -> CM/support -> reconstruction -> authority branch -> physics/readback -> rendering

The decomposition should not reintroduce architectural mixing that earlier documents were designed to avoid.

---

## 4. Top-Level MVC Module Families

At the highest level, the modules of the project should be grouped into three families:

* **Controller-side modules**
* **Model-side modules**
* **View-side modules**

### 4.1 Controller-Side Family

This family receives:

* user intent
* external commands
* requested perturbations

It does not own locomotion truth or render geometry.

### 4.2 Model-Side Family

This family owns:

* simulation truth
* body state
* locomotion logic
* physical state
* support state
* procedural targets

This is the largest and most important family.

### 4.3 View-Side Family

This family owns:

* screen conversion
* visible curve generation
* SDL drawing
* debug visualization

It must consume model state, not redefine it.

---

## 5. Controller-Side Modules

## 5.1 Input Controller

### Responsibility

The Input Controller reads external requests and converts them into internal commands.

### Typical inputs

* keyboard input
* mouse input if needed
* scripted test events

### Typical outputs

* locomotion requests
* crouch/jump requests
* perturbation requests
* debug toggles

### Important limitation

It should not directly manipulate:

* Box2D bodies
* spline rendering
* the internal support state

It only creates requests.

## 5.2 Perturbation Controller

### Responsibility

This module triggers external disturbances used for testing and interaction.

### Typical outputs

* push impulses
* impact requests
* environmental disturbance events

### Why it should be separate

The project explicitly studies perturbation and recovery.
So perturbation injection should not be hidden as an ad hoc helper inside unrelated code.

---

## 6. Core Model State Modules

## 6.1 Character State

### Responsibility

This is the main high-level state container of the character.

It should contain or reference:

* current hybrid regime
* current locomotion mode
* current gait phase
* current groundedness
* current recoverability classification
* the currently authoritative pose source

### Why it matters

This module acts as the central state hub through which the rest of the Model communicates.

It is not itself the place where all computation happens.

## 6.2 Stickman Geometry State

### Responsibility

This module owns the abstract stickman geometry defined in [01_Stickman_Model.md](01_Stickman_Model.md).

It should contain:

* node positions
* segment lengths
* nominal body parameters
* structural hierarchy data

### Why it should be explicit

The project distinguishes the abstract stickman from the physical body.
So this geometry state should not be dissolved into the Box2D module.

## 6.3 Procedural Pose State

### Responsibility

This module stores the current procedural articulated target.

It should contain:

* target node positions
* target joint angles
* target trunk orientation
* target arm and leg configurations

### Why it matters

It is the authoritative pose state during nominal procedural motion.

## 6.4 Previous Procedural State

### Responsibility

This module stores the immediately previous procedural state.

It exists to support:

* velocity estimation during procedural-to-physical transition
* temporal consistency in reconstruction
* branch selection in analytic IK

### Why it should be explicit

This is no longer an implementation detail.
The recent design documents now depend on it conceptually.

## 6.5 CM State

### Responsibility

This module stores the current CM-side truth used by the control stack.

For V1, it should be split explicitly into:

* `ProceduralCMState`
* `PhysicalCMState`

### Procedural CM State

This sub-state should contain:

* `cmTargetPosition`
* `cmTargetVelocity`
* `cmOperatingHeight`
* `pelvisOffsetTarget`
* `trunkLeanTarget`
* `airborneState`

### Physical CM State

This sub-state should contain:

* emergent CM position
* emergent CM velocity
* emergent CM operating height estimate if needed
* validity information from the last physical readback

### Why the split is required

The project explicitly distinguishes:

* a procedural CM target used during nominal motion
* an emergent CM obtained from the physical body when Box2D is authoritative

These are two different truths and should not be merged into one ambiguous structure.

## 6.6 Support State

### Responsibility

This module stores support-related information.

It should contain:

* current support mode
* current `SupportInterval`
* active support foot or feet
* current contact-derived support validity

### Why it matters

Support reasoning is central to walking, recovery, and falling.

## 6.7 Terrain State

### Responsibility

This module stores the environment information relevant to locomotion.

For V1, it may remain simple, but it should conceptually own:

* ground geometry
* support surface queries
* foothold validity queries

This keeps the code extensible toward uneven terrain later.

---

## 7. Control and Decision Modules

## 7.1 Intent Module

### Responsibility

This module converts user or scenario requests into a locomotion intent meaningful to the character.

### Outputs

* stand
* walk
* run
* jump
* crouch
* get-up

### Dependency rule

It may depend on input requests.
It should not depend directly on rendering or Box2D details.

## 7.2 Regime Manager

### Responsibility

This module decides which hybrid regime is active.

### Inputs

* current intent
* current recoverability
* current groundedness
* current support state
* transition progress state

### Outputs

* active hybrid regime
* transition flags
* `AuthorityPolicy`

### V1 AuthorityPolicy Type

For V1, `AuthorityPolicy` should be an explicit enum with values such as:

* `PROCEDURAL`
* `ENTERING_PHYSICAL`
* `PHYSICAL`
* `EXITING_PHYSICAL`

### Why it should be separate

This is the module that turns the project from “one locomotion controller” into a real hybrid system.

## 7.3 Gait Phase Module

### Responsibility

This module updates and stores the current gait phase.

For V1, it should use the discrete phase model defined in [12_Update_Loop_And_Timing.md](12_Update_Loop_And_Timing.md):

* `DOUBLE_SUPPORT`
* `STANCE_LEFT`
* `STANCE_RIGHT`
* `FLIGHT`

### Inputs

For V1, it should read:

* support/contact results from `SupportAndContactReasoner`
* nominal locomotion intent from `IntentModule`
* `airborneState` from `CMController`
* optional nominal timing progress when contacts alone are not sufficient

### Outputs

* current `GaitPhase`

### Why it should be separate

Gait phase is used by:

* CM control
* reconstruction
* foot placement
* nominal jump/flight logic

So it should not be buried in one of those modules.

## 7.4 CM Controller

### Responsibility

This module computes the target global body motion from locomotion intent and current regime.

### Inputs

* current intent
* current hybrid regime
* support state
* gait phase

### Outputs

* `cmTargetPosition`
* `cmTargetVelocity`
* `pelvisOffsetTarget`
* `trunkLeanTarget`
* `airborneState`

### Why the output matters

The reconstruction module now depends explicitly on this interface.

## 7.5 Support and Contact Reasoner

### Responsibility

This module interprets contact and support information.

### Inputs

* terrain state
* contact state
* physical readback state
* current foot states

### Outputs

* support mode
* support interval
* groundedness
* support validity

### Why it should be separate

This module is not the same thing as:

* Box2D contact callbacks
* recovery classification
* foot planning

It interprets support, but does not decide everything else.

## 7.6 Recovery Metric

### Responsibility

This module computes the disturbed-state class defined in [11_Recovery_Metrics.md](11_Recovery_Metrics.md).

### Inputs

* CM state
* support interval
* reachable next support region
* current body state

### Outputs

* `IN_SUPPORT_RECOVERABLE`
* `ONE_STEP_RECOVERABLE`
* `REPEATED_STEP_RECOVERABLE`
* `UNRECOVERABLE`

### Important note

This module is analytical and decision-oriented.
It should not itself trigger animations or physics.

## 7.7 Foot Placement Planner

### Responsibility

This module chooses the next support target for the swing foot when locomotion or recovery requires it.

### Inputs

* current CM state
* support state
* recovery classification
* terrain state
* gait phase

### Outputs

* `FootTarget` for nominal swing
* `FootTarget` for recovery stepping

### V1 FootTarget Type

For V1, `FootTarget` should be a small explicit data object containing at least:

* target planar position
* target foot orientation
* target support intent (`SWING`, `STANCE_PREP`, or equivalent)

---

## 8. Reconstruction and Motion-Authority Modules

## 8.1 Reconstruction and IK Module

### Responsibility

This module converts CM/support goals into a procedural articulated target pose.

### Inputs

* CM controller outputs
* support state
* foot placement targets
* gait phase
* current hybrid regime

### Outputs

* `ProceduralPose`

### V1 ProceduralPose Type

For V1, `ProceduralPose` should be the explicit output type of reconstruction.
It should contain at least:

* procedural joint targets
* procedural node targets
* trunk configuration
* leg target configurations
* arm target configurations

### Why it should be separate

It is the bridge from global locomotion logic to articulated posture.

## 8.2 Get-Up Controller

### Responsibility

This module owns the V1 procedural get-up behavior as a dedicated hybrid-regime module.

### Inputs

* current hybrid regime
* grounded stabilization status
* current grounded body configuration
* physical pose snapshot at get-up entry

### Outputs

* `GetUpTarget`
* current get-up sequence phase
* get-up completion status

### V1 GetUpTarget Type

For V1, `GetUpTarget` should contain at minimum:

* target joint configuration for the current get-up phase
* get-up operating height target
* trunk orientation target

### V1 scope

For V1, this module is allowed to support only a narrow set of grounded entry configurations.
It is not required to solve arbitrary stand-up recovery from every possible fallen pose.

### Why it should exist as a real module

Get-up is already a first-class regime in the design dossier.
So it should not remain hidden as an unnamed branch inside `ReconstructionIK`.

## 8.3 Transition Synchronizer

### Responsibility

This module handles state transfer between procedural and physical authority.

### Inputs

For V1, it should read:

* `PreviousProceduralState`
* current `ProceduralPoseState`
* access to `Box2DWorldAdapter` for procedural-to-physical transfer
* `PhysicalReadback` results for physical-to-procedural resynchronization

### Main duties

* procedural -> Box2D synchronization
* initialization of physical velocities from previous procedural state
* physical -> procedural resynchronization

### Why it must be separate

This transition logic is too important to remain implicit inside:

* the update loop
* the Box2D wrapper
* or the reconstruction module

It is a distinct hybrid-system responsibility.

---

## 9. Physical Simulation Modules

## 9.1 Physical Body Module

### Responsibility

This module owns the physical articulated body definition described in [09_Physical_Body_Model.md](09_Physical_Body_Model.md).

It should contain:

* physical segment definitions
* joint definitions
* collision shape definitions
* mass and inertia setup
* logical physical-segment identifiers used by the rest of the Model

### Important note

This module defines the structure of the body.
It is not the same thing as the world simulation stepper.
For V1, it should be treated as a body blueprint/configuration module, not as the owner of runtime `b2Body` objects.

## 9.2 Box2D World Adapter

### Responsibility

This module owns and operates the Box2D world.

It should handle:

* world creation
* rigid body creation
* joint creation
* stepping the world
* ownership of runtime `b2World`, `b2Body`, and `b2Joint` objects
* mapping logical body identifiers to runtime Box2D handles

### Why it should be separate

The project should not spread Box2D-specific code through the whole model layer.

### Ownership decision for V1

For V1, `Box2DWorldAdapter` is the runtime owner of:

* `b2World`
* all instantiated `b2Body*`
* all instantiated `b2Joint*`

`PhysicalBody` only defines what should be built.
It does not own the live Box2D objects.

## 9.3 Contact Collector

### Responsibility

This module gathers contacts generated by Box2D and exposes them safely to the rest of the Model after stepping.

### Why it matters

The dossier already established that contact events should not directly mutate high-level logic during solver execution.

So the contact collection mechanism deserves explicit module status.

## 9.4 Physical Readback Module

### Responsibility

This module converts Box2D post-step state into model-side physical state.

It should produce:

* body poses
* body velocities
* contact-relevant state
* groundedness-relevant state
* `PhysicalCMState`

### Why it should be separate

Reading back physics state is conceptually different from stepping physics and from interpreting support.

### Emergent CM responsibility

For V1, this module should also compute the emergent physical CM from:

* the runtime body transforms read from `Box2DWorldAdapter`
* the mass information defined by `PhysicalBody`

This makes `PhysicalReadback` the explicit producer of `PhysicalCMState`.

---

## 10. View-Side Modules

## 10.1 Render State Adapter

### Responsibility

This module transforms simulation-side state into the data the View needs.

It should gather:

* visible node positions
* trunk and head anchors
* debug markers
* support indicators

### Why it exists

The View should not directly crawl through the whole simulation model in an ad hoc way.

## 10.2 Continuous-Line Builder

### Responsibility

This module converts the visible stickman state into curve or spline control data.

### Outputs

* visual anchors
* spline control points
* stroke geometry instructions

### Why it matters

The project does not render a raw skeleton.
So this transformation is a real responsibility.

## 10.3 SDL Renderer

### Responsibility

This module owns SDL drawing and frame presentation.

It should handle:

* window management
* screen-space conversion
* final line drawing
* frame presentation

## 10.4 Debug Renderer

### Responsibility

This module renders optional debug visuals such as:

* raw nodes
* CM marker
* support interval
* regime label
* contact hints

This module is useful enough to deserve separation from the final-character renderer.

---

## 11. Timing and Orchestration Modules

## 11.1 Simulation Loop Module

### Responsibility

This module runs the fixed-step simulation logic defined in [12_Update_Loop_And_Timing.md](12_Update_Loop_And_Timing.md).

It should handle:

* time accumulation
* fixed-step execution
* ordering of module updates

### Why it matters

The update loop is now architecturally important enough that it should not remain embedded in `main` as a pile of inline logic.

## 11.2 Authority Branch Coordinator

### Responsibility

This module determines, at each simulation step, whether:

* procedural authority applies
* physical authority applies
* a transition synchronizer must run first
* get-up authority is active within the procedural branch

### Why it should be explicit

This is one of the core V1 design choices.
It should not remain an undocumented `if/else` spread across modules.

---

## 12. Dependency Rules

The following dependency rules should hold.

### 12.1 Allowed Direction

The intended direction is:

* input -> intent
* intent -> regime / gait / CM
* support/contact + recovery -> regime and planning
* CM / support / foot planning -> reconstruction
* authority coordination -> procedural or physical state update
* simulation state -> view adapters -> SDL renderer

### 12.2 Forbidden Couplings

The following couplings should be avoided:

* SDL modules depending on Box2D internals
* reconstruction directly stepping physics
* Box2D callbacks directly changing the active regime
* rendering modules owning locomotion truth
* input logic directly mutating body geometry or support state

These are exactly the kinds of mistakes that would later make the class diagram misleading.

---

## 13. Minimum V1 Module Set

If V1 needs a smaller implementation subset, the minimal useful module set is:

* `InputController`
* `CharacterState`
* `CMState`
* `SupportState`
* `IntentModule`
* `RegimeManager`
* `GaitPhaseModule`
* `CMController`
* `SupportAndContactReasoner`
* `RecoveryMetric`
* `FootPlacementPlanner`
* `ReconstructionIK`
* `GetUpController`
* `TransitionSynchronizer`
* `PhysicalBody`
* `Box2DWorldAdapter`
* `ContactCollector`
* `PhysicalReadback`
* `SimulationLoop`
* `RenderStateAdapter`
* `ContinuousLineBuilder`
* `SDLRenderer`

This set is large enough to preserve the architecture, but still small enough for a V1 codebase.

---

## 14. What This Document Enables Next

Once these modules are accepted, the next software design step becomes much clearer:

* decide which modules become one class
* decide which modules are better expressed as services or state objects
* decide constructor dependencies
* draw the class diagram

So this document is intended to make the future class diagram mostly mechanical rather than speculative.

---

## 15. Summary of Main Takeaways

* The project now has a module-level structure that respects the earlier MVC and hybrid-control documents.
* State modules, decision modules, reconstruction modules, physical modules, timing modules, and view modules should remain distinct.
* `PreviousProceduralState`, `RecoveryMetric`, `GetUpController`, `TransitionSynchronizer`, and `AuthorityBranchCoordinator` are now explicit architectural modules rather than hidden details.
* `Box2DWorldAdapter` owns the live Box2D objects, while `PhysicalBody` remains the physical-body blueprint and `PhysicalReadback` produces the emergent physical CM state.
* This document is the last abstraction layer needed before the future class diagram.
