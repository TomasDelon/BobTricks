# Update Loop and Timing

## Implementation Alignment Note

This document remains valid for **temporal ordering principles**.

For concrete midpoint implementation, the following documents take precedence:

- [`../implementation/01_Runtime_Architecture.md`](../implementation/01_Runtime_Architecture.md)
- [`../implementation/05_Midpoint_Procedural_Locomotion_Spec.md`](../implementation/05_Midpoint_Procedural_Locomotion_Spec.md)

Important midpoint interpretation:

- midpoint uses a fixed-step loop at `60 Hz`
- midpoint gait progression is timer-driven
- midpoint does not activate the physical authority branch
- `GaitPhase` remains a dedicated support-cycle type

---

## 1. Role of This Document

This document defines the **temporal organization** of the project.

The previous documents already established:

* the body model
* the CM-centered locomotion logic
* the hybrid regime structure
* the control architecture
* the physical body model
* the reconstruction and IK logic
* the recovery metric

What is still missing is the answer to this question:

**in what order do all these systems run over time?**

This document therefore defines:

* the main update loop
* the distinction between simulation time and rendering time
* the fixed-step simulation policy
* the order in which control, physics, contacts, and regime transitions should be processed

This document does **not** yet define:

* exact APIs
* class ownership
* threading
* final optimization strategies

Those belong to later documents.

---

## 2. Why Timing Must Be Defined Explicitly

The project combines:

* procedural nominal motion
* state reconstruction
* physical simulation in Box2D
* recovery evaluation
* rendering in SDL

These systems cannot safely be left in an unspecified temporal order.

If the update order is ambiguous, the project will quickly suffer from:

* regime-switch glitches
* contact handling errors
* inconsistent use of stale state
* unstable control behavior
* rendering one state while simulating another

This is especially important because the project is hybrid:

* in nominal motion, pose reconstruction drives the visible character
* in physical regimes, Box2D becomes the authoritative source of body evolution

So timing is not only an implementation detail.
It is part of the architecture itself.

---

## 3. Timing Principles for V1

V1 should follow four main timing principles.

### 3.1 Fixed Simulation Time Step

The simulation should run with a **fixed time step**.

This is strongly preferable to a variable-step simulation because:

* physics behavior becomes easier to tune
* regime thresholds become more interpretable
* reconstruction and recovery logic become more predictable
* Box2D behaves more consistently

### 3.2 Rendering Must Be Decoupled from Simulation

Rendering should not define simulation timing.

The screen refresh rate may vary, but the internal simulation should remain stable.

So the project should conceptually separate:

* **simulation updates**
* **render presentation**

### 3.3 One Authoritative State per Simulation Step

At each simulation step, the system must know which state is authoritative.

For V1:

* in nominal procedural regimes, the procedural model and reconstructed pose are authoritative
* in physical regimes, the Box2D state is authoritative

This prevents the system from mixing two incompatible truths at the same time.

### 3.4 Contacts Must Be Processed in a Safe Phase

Contact events generated during physics stepping should not immediately mutate the control architecture in the middle of the same solver step.

Instead:

* Box2D should step first
* contact results should be collected
* higher-level logic should process them afterward

This keeps the system temporally coherent.

---

## 4. Fixed-Step Policy

For V1, the simulation should use a fixed update interval, conceptually something like:

* `dt_sim = 1 / 60 s`

The exact value can still be tuned later, but the policy should be fixed-step from the beginning.

### 4.1 Why 60 Hz Is a Reasonable Baseline

It is a common compromise between:

* responsiveness
* numerical stability
* implementation simplicity

It also aligns reasonably well with:

* real-time SDL rendering
* Box2D usage patterns
* controller tuning expectations

### 4.2 Relation to Stable Control Literature

Discrete-time control quality depends strongly on time-step choice.
This is one of the central messages of [Stable Proportional-Derivative Controllers](../references/papers/Stable%20Proportional-Derivative%20Controllers.pdf), which explicitly discusses stability in discrete simulation.

Even though V1 does not yet finalize its control law, this literature still supports a cautious timing policy:

* stable fixed-step simulation first
* controller tuning second

---

## 5. Simulation Time vs Render Time

The project should treat simulation time and render time as related but distinct.

### 5.1 Simulation Time

Simulation time is the time used by:

* procedural locomotion logic
* reconstruction and IK
* recovery evaluation
* Box2D stepping

### 5.2 Render Time

Render time is the time used by:

* SDL event polling
* frame presentation
* curve and spline drawing
* debug overlays

### 5.3 Why the Distinction Matters

If rendering directly controls simulation:

* the physics may behave differently on slow and fast machines
* recovery thresholds may become frame-rate dependent
* the visible result may jitter under irregular frame pacing

So V1 should treat rendering as a consumer of simulation state, not as the owner of simulation timing.

---

## 6. High-Level Frame Structure

At the highest level, each application frame should conceptually follow this order:

1. poll input and external events
2. accumulate real elapsed time
3. run zero or more fixed simulation steps as needed
4. render the latest available state

This means that:

* one render frame may contain zero, one, or multiple simulation steps
* simulation remains stable even if rendering rate fluctuates

This is the standard logic needed for a robust real-time hybrid simulation.

---

## 7. Main Simulation Step Order

Each **simulation step** should conceptually follow this order.

### 7.1 Read Intent and External Requests

The system first reads:

* user commands
* scripted requests
* requested perturbations

These requests are interpreted by the Controller and made available to the Model.

### 7.2 Evaluate High-Level Intent

The intent layer updates the desired high-level action, such as:

* stand
* walk
* run
* jump
* recover
* get up

### 7.3 Evaluate the Current Hybrid Regime Context

Before generating new targets, the system evaluates:

* current regime
* current groundedness
* previous recovery classification
* whether a transition is already in progress

This step should not yet finalize a new regime based on fresh physics contacts from the current step, because those contacts do not yet exist.

### 7.4 Update CM-Level Objectives

The CM controller updates:

* target CM behavior
* target operating height
* regime-specific global motion goals

This step uses the best currently known state before the next physical integration.

During nominal jump flight, this step is also responsible for integrating the ballistic CM trajectory.

In V1, reconstruction does not invent the airborne CM path.
The CM controller must provide it.

### 7.5 Update Support and Foot Placement Logic

The system then updates:

* support interpretation
* nominal foot placement planning
* recovery-step intent if applicable

In nominal motion, this is mainly procedural.
In recovery-related contexts, this may also consult the last known disturbed state.

### 7.6 Reconstruct the Procedural Target Pose

The reconstruction layer produces:

* target leg configurations
* trunk orientation
* arm posture
* overall procedural articulated target

This step uses the current regime, CM objectives, and support goals.

### 7.7 Apply the Step According to the Active Authority

This is where the hybrid architecture branches.

#### Procedural / Kinematic Authority

If the current regime is nominal procedural:

* the reconstructed pose becomes the visible and simulation-authoritative state for this step
* the physical Box2D body is not the primary driver of locomotion
* the Box2D body is kept in kinematic sync with the procedural pose

This means that, in V1 nominal motion:

* the procedural state is authoritative
* the Box2D articulated body remains synchronized but non-authoritative
* the synchronized state is available if a transition to physical authority is triggered

#### 7.7a Kinematic Sync in Nominal Motion

When the regime remains nominal, the project should:

* write the reconstructed articulated pose into the procedural state
* synchronize the Box2D body representation to that pose as a kinematic state
* store the current and previous procedural states for later velocity estimation if a physical transition is triggered

#### Physical Authority

If the current regime is physical, falling, or grounded stabilization:

* control actions are prepared for the physical body
* Box2D will become authoritative after integration

This branching is one of the most important timing decisions in the project.

#### 7.7b Entering Physical Authority

If this is the first simulation step in which the regime becomes physical, the system must first:

* initialize the dynamic Box2D state from the current procedural articulated state
* estimate initial linear and angular velocities from the stored previous procedural state
* transfer the resulting synchronized body into physical authority before stepping Box2D

### 7.8 Step Box2D When the Physical Regime Is Active

If the physical regime is active, the project steps Box2D once for this simulation step.

At that time, Box2D computes:

* rigid body integration
* collisions
* contact impulses
* joint responses

This physical step must happen before any new decision is made from the resulting contacts.

### 7.9 Read Back Physical Results

After the Box2D step, the project reads back:

* rigid body poses
* velocities
* contact states
* groundedness-relevant information

This is how the Model updates its physical-side truth.

### 7.10 Evaluate Recovery and Regime Transitions

Only after the state has been updated should the project evaluate:

* recoverability class
* need for a recovery step
* transition to falling
* grounded stabilization entry
* readiness to re-enter procedural control

This ordering matters because the recovery metric should work on the latest valid state, not on speculative pre-step information.

### 7.11 Finalize the Authoritative Model State

At the end of the simulation step, the Model stores the final state that the View will use:

* procedural pose if the regime stayed nominal
* physical body state if the regime is physical-side
* synchronized state if a transition finished

This closes the simulation step cleanly.

---

## 8. Why Contacts Must Be Processed After Physics Stepping

The timing of contact handling deserves explicit treatment.

### 8.1 Contact Generation

During the Box2D step, contacts are generated and resolved as part of the physics solver.

### 8.2 Architectural Rule

Higher-level project logic should **not** directly reconfigure the regime manager or reconstruction system in the middle of that solver step.

Instead, V1 should use this safer rule:

* Box2D generates and resolves contacts
* contact events are recorded
* the project processes them after the step

### 8.3 Why This Rule Matters

This avoids:

* mixing solver internals with higher-level logic
* inconsistent mid-step state changes
* brittle transition timing

It also makes the update loop easier to reason about and document.

---

## 9. Procedural-to-Physical Transition Timing

Because V1 uses a switch-based hybrid architecture, the timing of transitions matters a lot.

### 9.1 Entering Physical Regime

When the system leaves nominal procedural control:

* the physical body must be initialized from the current procedural state
* the current posture must be transferred as faithfully as possible
* any immediately relevant velocities should be estimated from the stored previous procedural state

This transition should happen at a clean simulation-step boundary, not halfway through a frame.

### 9.2 Physical Evolution

Once the physical regime is active:

* Box2D becomes authoritative
* the recovery metric and regime logic operate on the physical readback state

### 9.3 Returning Toward Procedural Motion

When the system leaves physical authority:

* it should not snap directly to an unrelated nominal pose
* it should first capture the current physical state
* then reconstruct a compatible procedural re-entry target

This timing logic is one of the direct consequences of [06_Transitions_And_Continuity.md](06_Transitions_And_Continuity.md).

---

## 10. Relation to Composite and State-Dependent Control

[Composite Control of Physically Simulated Characters](../references/papers/Composite%20Control%20of%20Physically%20Simulated%20Characters.pdf) is useful here because it emphasizes a crucial point:

* transitions and blends should depend on the current state, not only on fixed timing

This supports a general principle for the project:

* timing alone should not decide recovery or failure
* time organizes the loop
* state decides what the loop does

So this document defines temporal ordering, but not a purely time-driven controller.

---

## 11. Sub-Stepping Policy

V1 should keep sub-stepping simple.

### 11.1 Preferred V1 Rule

The default should be:

* one main fixed simulation step
* one Box2D step per simulation step

### 11.2 When Sub-Stepping May Be Introduced Later

If later testing shows that contacts or impacts require finer integration, the project may introduce:

* multiple physics substeps inside one logical simulation step

But this should be treated as a later refinement, not as a design dependency from day one.

This keeps V1 simpler and easier to debug.

---

## 12. Gait Phase and Timing

The timing layer must also support gait-phase progression.

For V1, gait phase should be represented explicitly as a **discrete state enum**, not as an unspecified abstract value.

A reasonable V1 baseline is:

```text
GaitPhase =
  DOUBLE_SUPPORT
  STANCE_LEFT
  STANCE_RIGHT
  FLIGHT
```

with the following semantics:

* `DOUBLE_SUPPORT`: both feet contribute to support
* `STANCE_LEFT`: left foot supports, right foot swings
* `STANCE_RIGHT`: right foot supports, left foot swings
* `FLIGHT`: no foot is currently supporting the body

This choice is appropriate for V1 because it is:

* simple
* readable
* directly usable by reconstruction and support logic

The architectural point then becomes:

* phase must update once per simulation step
* phase progression must use simulation time, not render time
* recovery and falling may interrupt or override nominal phase progression

This ensures that gait logic remains temporally coherent.

---

## 13. Timing Failure Modes to Avoid

The main timing mistakes to avoid are:

* using variable simulation time steps tied directly to rendering
* making regime changes in the middle of a physics step
* reading contacts before the authoritative physical update is complete
* rendering stale state after a different state has become authoritative
* mixing procedural and physical authority in the same step without a clear rule
* using render-frame timing to drive gait phase or recovery thresholds

These are architectural mistakes, not only implementation bugs.

---

## 14. What Is Deferred to Later Documents

This document does not yet define:

* the exact accumulator implementation
* the exact contact-listener API
* exact Box2D iteration counts
* interpolation between simulation states for rendering
* whether future versions will use multi-threading

Those elements belong to later software design and implementation documents.

---

## 15. Summary of Main Takeaways

* V1 should use a fixed simulation time step, independent of rendering.
* SDL rendering consumes simulation state; it does not define simulation timing.
* Each simulation step follows a clear order: intent, CM/support update, reconstruction, authority branch, physics step if needed, readback, recovery evaluation, finalize state.
* Contacts should be interpreted after the Box2D step, not as immediate regime-changing logic during the solver step.
* Procedural-to-physical and physical-to-procedural transitions should occur at clean simulation-step boundaries.
* Time organizes the loop, but state still decides the control regime and recovery behavior.
