# Assumptions and Open Questions

## 1. Role of This Document

This document separates three things that should not be mixed:

* what the project has already decided
* what the project is explicitly simplifying for V1
* what still remains open and must be resolved later

The previous documents already define the intended system in substantial detail.
At this stage, the main danger is no longer “having no idea”.
The main danger is confusing:

* settled architecture
* temporary simplifications
* unresolved technical questions

This document exists to prevent that confusion.

It should also make it easier to prepare:

* the future class diagram
* the future `cahier des charges`
* the implementation TODO list

---

## 2. Why This Separation Is Necessary

A design dossier becomes hard to use when it does not distinguish between:

* assumptions adopted on purpose
* details deferred to later
* things that are still genuinely uncertain

This project especially needs that distinction because it mixes:

* biomechanics-inspired locomotion
* procedural reconstruction
* Box2D physical response
* hybrid regime switching
* SDL rendering

Without a dedicated document for assumptions and open questions, the implementation phase would risk:

* arguing again about already settled design decisions
* treating temporary simplifications as permanent truth
* overlooking unresolved issues until they become blockers

---

## 3. Major Design Decisions Already Fixed

The following points should be considered **closed for V1** unless a later document explicitly revises them.

### 3.1 Project Scope

V1 targets:

* a 2D stickman
* procedural nominal locomotion
* hybrid physical reaction under perturbation
* recovery when possible
* falling when necessary
* grounded stabilization
* procedural get-up

### 3.2 Architectural Style

The project uses:

* C++
* SDL
* Box2D
* MVC

### 3.3 Hybrid Character Architecture

V1 will use:

* **procedural / kinematic nominal motion**
* **physical Box2D authority during perturbation and related reactive regimes**

The project will **not** use an always-physical character architecture in V1.

### 3.4 Physical Body Presence in Nominal Motion

The Box2D articulated body will still exist in nominal motion.

It will be kept in:

* **kinematic sync**

rather than disabled or created only on demand.

### 3.5 Support Geometry

V1 uses:

* finite simple feet

and not:

* point feet as the final support model
* full heel/toe segmented support

### 3.6 Reconstruction Strategy

V1 reconstruction uses:

* task-priority logic
* analytic 2D IK for two-link limbs
* regime-dependent heuristics for trunk and arms

and does not use a full whole-body optimization stack in V1.

### 3.7 Recovery Metric Strategy

V1 recovery uses:

* a capture-inspired operational metric
* `SupportInterval`
* a simple `Capture Point` formulation
* one-step recovery as the main explicit reactive target
* repeated-step recovery as repeated one-step re-evaluation

### 3.8 Timing Policy

V1 uses:

* fixed simulation step
* render decoupled from simulation
* Box2D contacts interpreted after stepping
* clean step-boundary transitions between procedural and physical authority

### 3.9 Gait Phase Representation

V1 uses a discrete gait-phase enum:

* `DOUBLE_SUPPORT`
* `STANCE_LEFT`
* `STANCE_RIGHT`
* `FLIGHT`

---

## 4. Explicit V1 Simplifications

The following simplifications are intentional.
They are not mistakes.

### 4.1 Simplified Anatomy

The stickman is not a full human anatomical model.

It intentionally uses:

* a minimal visible node set
* no explicit anatomical shoulders or hips in the visible graph
* simplified segment structure

### 4.2 Simplified Physics

The physical body is a reduced Box2D articulation, not a biomechanical human body.

This means:

* simplified mass distribution
* simplified inertias
* simplified collision geometry
* simplified support shapes

### 4.3 Simplified Recovery Logic

V1 does not implement:

* full N-step capturability
* full MPC
* full optimal stepping

It implements a simpler decision framework that is still literature-informed.

### 4.4 Simplified IK

V1 does not implement:

* generic constrained whole-body IK with full priority enforcement

It instead uses:

* explicit priorities
* analytic limb IK
* heuristic posture composition

### 4.5 Simplified Terrain

V1 should remain extensible toward rough, sloped, or discontinuous terrain, but it does not need to solve advanced terrain adaptation fully.

### 4.6 Simplified Get-Up

V1 get-up is not assumed to be a full generalized “stand up from any arbitrary fallen configuration” solver.

It is allowed to start with a narrower set of grounded configurations.
For planning purposes, V1 should be kept to at most one or two canonical grounded entry classes, not an open-ended family of arbitrary floor poses.

---

## 5. Assumptions About the Character and World

The current design assumes the following.

### 5.1 World Assumptions

The character evolves in:

* a planar 2D locomotion world
* gravity-dominated motion
* support surfaces that can be represented meaningfully in 2D

### 5.2 Character Assumptions

The character is assumed to have:

* a nominal upright configuration
* meaningful left/right support alternation
* limb lengths that remain fixed in reconstruction
* useful support through feet rather than through arbitrary body points during normal locomotion

### 5.3 Control Assumptions

The current design assumes:

* CM-level reasoning is useful
* support reasoning is necessary
* reconstruction should respect support first and style later
* recovery should be state-driven rather than purely scripted

---

## 6. Questions Already Answered

These questions should no longer be treated as open:

* Should V1 be always-physical?  
  No.

* Should V1 use Box2D?  
  Yes.

* Should the physical body exist in nominal motion?  
  Yes, in kinematic sync.

* Should feet be explicit physical bodies?  
  Yes.

* Should the recovery metric use support and CM logic rather than appearance alone?  
  Yes.

* Should nominal jump flight be integrated by the reconstruction layer?  
  No. The CM controller owns the ballistic CM trajectory.

These answers should now be treated as settled architecture.

---

## 7. Main Open Questions Still Remaining

The following points are still genuinely open.

### 7.1 Exact Joint Limit Values

The dossier establishes that joint limits are mandatory, but it still does not fix:

* exact numeric angular ranges
* whether all joints use symmetric or asymmetric limits

### 7.2 Exact Foot Dimensions

The dossier fixes the existence of a finite foot, but not yet:

* exact foot length relative to `L_rest`
* exact foot thickness
* exact ankle-to-foot local offset

### 7.3 Exact Mass Distribution

The physical model requires mass and inertia, but V1 still has to choose:

* how total mass is divided among torso, limbs, and feet
* how simplified inertias are assigned

This question is not isolated.
It directly affects the physical CM estimate and therefore interacts with the numerical behavior of the recovery metric.

### 7.4 Exact Recovery Thresholds

The logic of `in-support recoverable`, `one-step recoverable`, and `unrecoverable` is defined.
But the exact thresholds are still open, for example:

* safety margins on support intervals
* maximum step length used by the recovery metric
* allowed divergence before a repeated-step attempt is abandoned

These thresholds should be tuned together with mass distribution and CM-height assumptions, not as an unrelated later pass.

### 7.5 Exact Get-Up Entry Conditions

The dossier supports get-up conceptually, but still leaves open:

* which grounded states are valid get-up starting states
* how many get-up entry poses V1 will support
* when grounded stabilization is considered complete enough to start get-up

### 7.6 Exact Contact Filtering Strategy

The dossier says that self-collision should be mostly ignored in V1, but the precise filter matrix is still open.

### 7.7 Exact Debug Set for V1

The rendering architecture supports debug overlays, but it is still open which debug views are mandatory for development.

This matters because some of them are nearly essential:

* CM marker
* support interval
* current regime
* current gait phase

---

## 8. Open Questions That Should Be Answered Before Implementation Deepens

Some open questions are more urgent than others.

The following should be answered early in implementation:

### 8.1 What exact foot dimensions should V1 use?

Because support, recovery, and contact all depend on it.

### 8.2 What exact joint limits should be used?

Because reconstruction, Box2D stability, and falling behavior all depend on them.

### 8.3 What exact recovery thresholds should be tuned first?

Because regime switching depends on them.

### 8.4 What grounded states are acceptable for V1 get-up?

Because get-up scope can easily expand beyond what 2 months allows.
The preferred V1 answer is a deliberately narrow set of canonical entry states, not “any grounded pose”.

---

## 9. Risks for V1

These are not just abstract concerns.
They are real project risks.

### 9.1 Hybrid Transition Risk

The transition from procedural to physical authority may still look bad if:

* velocity estimation is poor
* the synchronized Box2D body is not aligned tightly enough
* the transition is triggered too late

### 9.2 Foot Contact Risk

Even with finite feet, contact may still be fragile if:

* the foot dimensions are badly chosen
* friction values are poorly tuned
* support interpretation is noisy

### 9.3 Recovery False Positives

The recovery metric may classify some states as recoverable when they are not truly recoverable in the V1 body and timing constraints.

### 9.4 Reconstruction Plausibility Risk

Even with closed-form leg IK, the overall posture may still look unnatural if:

* trunk offsets are badly chosen
* arms are too passive
* landing poses are too symmetrical or too stiff

### 9.5 Get-Up Scope Risk

Get-up is easy to underestimate.
If left unconstrained, it can consume too much time compared with walking, recovery, and falling.

### 9.6 Tuning Risk

The architecture is now clear, but V1 still depends on many tuned values:

* joint limits
* friction
* support margins
* step reachability
* locomotion offsets

This is a normal project risk, not a design failure.

---

## 10. What Is Out of Scope for V1

The following should be considered explicitly outside the scope of V1 unless time remains later.

* always-physical character architecture
* full heel/toe support progression
* full self-collision
* advanced uneven-terrain locomotion
* general optimal control or HQP whole-body solver
* generalized get-up from every possible ground pose
* full biomechanical validation against rich human datasets

These items may be future work.

---

## 11. What Must Be Preserved for Future Evolution

Even though V1 is simplified, the following future directions must remain possible:

* migration toward always-physical architecture
* richer support modeling
* stricter task-priority reconstruction
* better recovery metrics
* richer terrain adaptation
* improved get-up coverage

This means V1 should stay simple, but not corner itself into a dead-end structure.

---

## 12. Practical Reading of This Document

This document should be used as follows:

* if something is listed in Sections 3 or 6, treat it as decided
* if something is listed in Section 4, treat it as an intentional simplification
* if something is listed in Section 7 or 8, treat it as genuinely open
* if something is listed in Sections 9 or 10, treat it as a planning risk or scope limit

This is the intended working discipline for the next stage of the project.

---

## 13. Summary of Main Takeaways

* The dossier now contains a large number of settled design decisions that should no longer be reopened casually.
* V1 is intentionally simplified in anatomy, physics, recovery, IK, terrain, and get-up scope.
* Several implementation-relevant details remain open, especially joint limits, foot dimensions, mass distribution, recovery thresholds, and get-up entry conditions.
* The main risks for V1 are hybrid transition quality, contact quality, recovery tuning, reconstruction plausibility, and get-up scope.
* This document is meant to prevent confusion between settled architecture, acceptable simplification, and unresolved design work.
