# Transitions and Continuity

## 1. Role of This Document

This document defines how the project should connect its locomotion and control regimes **over time**.

The previous documents established:

* the body model
* the CM-centered locomotion logic
* the hybrid regime structure
* the main control layers
* the physical tracking layer

What is still missing is the logic that explains how the system should move **from one valid state to another** without producing unnecessary discontinuities, snaps, or visual breaks.

This document therefore answers the following questions:

* What kinds of continuity matter in this project?
* Which discontinuities are unacceptable, and which are physically legitimate?
* How should procedural regimes transition into other procedural regimes?
* How should the system transition from procedural control into physical reaction?
* How should the system later re-enter procedural control after a physical phase?
* How should contact continuity be preserved?
* How should all of this remain extensible to future terrain complexity?

This document does **not** yet define:

* exact numerical thresholds
* exact Box2D implementation details
* exact class interfaces
* full validation methodology

Those topics belong to later documents.

---

## 2. Why Continuity Matters

The project is not built from isolated animations.
It is built from **connected control regimes** that must produce believable motion as one continuous process.

Without adequate continuity:

* the CM appears to teleport or jitter
* support logic becomes unstable
* articulated reconstruction becomes visually noisy
* contact points slide or detach unnaturally
* perturbation and recovery stop looking physical

This is consistent with the lessons of [Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf), which emphasizes interpolation, synchronized blending, springs, and long transitions rather than abrupt pose replacement.

It is also consistent with [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), where locomotion is organized as controlled state transitions, often tied to meaningful events such as elapsed phase or foot contact, rather than arbitrary pose jumps.

---

## 3. Kinds of Continuity That Matter

Continuity in this project is not a single thing.
Several kinds of continuity must be distinguished.

### 3.1 CM Continuity

The most important continuity is the continuity of the **center of mass state**:

* position continuity
* velocity continuity
* when possible, acceleration continuity

This is critical because the CM organizes:

* global locomotion
* support interpretation
* recovery logic
* pose reconstruction

### 3.2 Support Continuity

The motion must also remain coherent with respect to support.

This means that transitions should respect:

* whether the character is in single support, double support, flight, or fall
* whether a support point is still valid
* whether a new support is being established

Support continuity is often more important than superficial pose continuity.

### 3.3 Contact Continuity

When a foot or other support-relevant body part is supposed to stay planted, the motion should preserve that condition.

This is strongly related to the lessons of [Footskate Cleanup for Motion Capture Editing](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Footskate%20cleanup%20for%20motion%20capture%20editing.pdf), which shows how easily realism is damaged when planted contacts are not enforced consistently.

### 3.4 Articulated Pose Continuity

Joint angles and node positions should not snap unnecessarily.
Even when global behavior changes, the articulated body should evolve through plausible intermediate states whenever possible.

### 3.5 Visual Continuity

Because the character will later be rendered as a continuous curve-like stickman, continuity is not only mechanical.
It is also visual.

This means that the internal state transitions must avoid producing:

* sudden silhouette breaks
* discontinuous body curvature
* visual popping at regime changes

---

## 4. Continuity Does Not Mean Perfect Smoothness Everywhere

Continuity is a goal, but not an absolute law.

Some events are physically expected to introduce sharper changes:

* landing impacts
* sudden collisions
* hard support redirection
* failed recovery
* first ground contact during a fall

This point is important.
The project should not try to hide every discontinuity with cosmetic filtering.

The correct distinction is:

* **bad discontinuity**: arbitrary snap caused by poor transition design
* **legitimate discontinuity**: physically motivated change caused by impact, contact, or loss of support

This is also consistent with [Kuo 2007](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Kuo2007-Six-Determinants-Gait-Inverted-Pendulum.pdf), which emphasizes that step-to-step walking transitions involve real redirection of motion rather than perfectly smooth cost-free continuation.

---

## 5. General Principles for Transition Design

Before discussing specific transition types, several general principles should be fixed.

### 5.1 Prefer State Continuity Over Pose Replacement

The system should prefer transitioning through compatible states rather than instantly replacing one pose by another.

This means transitions should be built around:

* CM state
* support state
* contact state
* body orientation
* current dynamic context

and not only around visual pose identity.

### 5.2 Prefer Event-Based Transition Boundaries

Whenever possible, regime changes should occur at meaningful motion events such as:

* foot contact
* support release
* take-off
* landing
* confirmed loss of recoverability
* regained stable support

This principle is strongly supported by [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), where many state transitions are tied to elapsed phase or foot contact.

### 5.3 Prefer Phase-Aware Transitions

For cyclic motions such as walking or running, two states should not be blended as if phase did not matter.

A transition should ideally respect:

* current gait phase
* support side
* current swing/stance assignment
* whether the body is near support exchange

This is also consistent with the synchronized blending ideas in [Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf).

### 5.4 Preserve What the Player or Viewer Will Read First

If perfect continuity cannot be preserved everywhere, the system should preserve first:

* support validity
* whole-body balance logic
* CM plausibility
* planted contact integrity

and only after that:

* secondary stylistic pose details

---

## 6. Main Tools for Achieving Continuity

Different transitions need different tools.
The project should not rely on a single universal method.

### 6.1 Direct Interpolation

Interpolation is useful when two states are already close and physically compatible.

It can be applied to:

* scalar parameters
* target heights
* locomotion parameters
* compatible poses

However, interpolation alone is not sufficient when support conditions change or when physical disturbance is large.

### 6.2 Polynomial Blending

Polynomial blending is useful when the system wants explicit continuity of:

* position
* velocity
* possibly acceleration

It is particularly relevant for:

* planned transitions between procedural states
* re-entry to procedural motion after recovery
* connecting nearby CM trajectories cleanly

This is one of the reasonable answers to the earlier question of how to connect two procedural phases whose curves do not join perfectly.

### 6.3 Spring-Damper Convergence

Springs and dampers are useful when the system should converge naturally toward a new target rather than snap directly to it.

This is especially useful for:

* crouch transitions
* soft recovery
* landing absorption
* physical-to-procedural re-synchronization

This approach is strongly compatible with the transition style suggested in [Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf).

### 6.4 Event-Gated Switching

Sometimes continuity is best preserved not by smoothing harder, but by switching only when the motion reaches a valid boundary.

Examples:

* switch walking controllers near support exchange
* enter landing logic at actual contact
* enter get-up only once grounded stabilization is real

### 6.5 State Capture and Re-Synchronization

When returning from a strongly physical regime, the system should not blindly resume the old procedural phase.
Instead, it should:

* capture the current physical state
* identify a compatible procedural entry point
* progressively synchronize back to that procedural regime

This idea is essential for believable recovery.

---

## 7. Procedural-to-Procedural Transitions

These are transitions where both sides remain under primarily procedural control.

Examples:

* stand to walk
* walk to run
* run to walk
* stand to crouch
* crouch to jump preparation
* walk to stop

### 7.1 What Should Stay Continuous

Whenever possible, these transitions should preserve:

* CM position continuity
* CM velocity continuity
* support logic continuity
* articulated pose continuity

### 7.2 What May Change

These transitions may legitimately change:

* operating CM height
* step frequency
* oscillation amplitude
* stance/swing timing
* target posture style

The important point is that those changes should be introduced gradually or at meaningful phase boundaries.

### 7.3 Recommended Transition Strategies

The main strategies here are:

* parameter blending
* phase-aware interpolation
* polynomial CM bridging
* spring convergence toward a new operating level

For example:

* stand to crouch is naturally compatible with spring-like vertical convergence
* walk to run is more naturally handled by phase-aware parameter transition than by naive pose interpolation

---

## 8. Procedural-to-Physical Transitions

These transitions occur when the nominal controller is no longer sufficient because physical disturbance becomes dominant.

Examples:

* large push during walking
* destabilizing landing
* collision with the environment
* sudden support loss

### 8.1 The Main Goal

The goal is not to preserve the old procedural curve at all costs.
The goal is to preserve **coherence** while allowing the physical event to take over.

### 8.2 What Should Be Preserved

At the moment of transition, the system should preserve as much as possible:

* current body state
* current support interpretation
* current contact status
* current CM position and velocity
* current phase context if still relevant

The physical regime should begin from the actual current state, not from a reset or predefined impact pose.

### 8.3 What Should Change

What changes is the interpretation of control:

* the procedural target loses authority
* physical response gains authority
* the CM becomes less prescribed and more emergent

This is the core of the hybrid architecture.

### 8.4 Why This Matters

If procedural-to-physical transition is badly handled, the viewer sees:

* a snap into ragdoll
* a frozen pose that suddenly collapses
* contact inconsistency
* a loss of whole-body logic

The project should avoid all of these.

---

## 9. Physical-to-Recovery and Physical-to-Procedural Re-Entry

These are among the hardest transitions in the whole system.

The problem is not just to stop the physical phase.
The problem is to decide **when** and **how** the body can again be treated as intentionally controlled.

### 9.1 Recovery Is Not Immediate Re-Entry

After a perturbation, the body may still be:

* rotating
* stepping
* dissipating excess motion
* searching for support

So the system should usually pass first through a recovery logic before returning to full procedural control.

### 9.2 Conditions for Re-Entry

Procedural re-entry should require at least a sufficiently compatible state such as:

* regained viable support
* bounded instability
* CM back in a capturable or controllable region
* no ongoing uncontrolled collapse
* a pose that can be matched to a procedural family

### 9.3 Re-Synchronization Strategy

Once re-entry becomes possible, the system should:

1. capture the current physical state
2. choose the appropriate procedural family
3. choose or infer a compatible procedural phase
4. blend or converge toward that phase without snap

This may be achieved with:

* parameter re-initialization
* spring convergence
* polynomial bridging of CM targets
* progressive strengthening of pose tracking

### 9.4 Why This Is Better Than Pose Snapping

Snapping directly to a canonical procedural pose would destroy:

* the evidence of the perturbation
* contact coherence
* dynamic plausibility
* recovery credibility

The system should therefore re-enter by **matching the current physical reality**, not by erasing it.

---

## 10. Falling, Grounding, and Get-Up Transitions

The same continuity logic also applies to failure.

### 10.1 Recovery-to-Fall

When recovery is no longer viable, the transition to falling should not be hidden.
It should be recognized as a real loss of recoverability.

This means:

* abandon impossible upright targets
* let the physical state continue consistently
* preserve whatever protective or stabilizing behavior still makes sense

### 10.2 Falling-to-Grounded Stabilization

This transition should occur only after the large-scale falling dynamics have sufficiently dissipated.

The system should not declare itself grounded too early.
It must wait until a real grounded configuration has emerged.

### 10.3 Grounded Stabilization-to-Get-Up

The system should only enter procedural get-up once:

* the body has a usable grounded pose
* impact chaos has ended
* a valid get-up family can be selected

This again favors state capture followed by controlled procedural re-entry.

---

## 11. Contact Continuity and Foot Stability

Contact continuity deserves its own section because it is one of the most visible sources of failure.

### 11.1 Planted Contacts Must Stay Planted

When a foot is acting as a support, the transition logic should avoid:

* arbitrary foot drift
* visible footskate
* support teleportation

This is directly motivated by [Footskate Cleanup for Motion Capture Editing](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Footskate%20cleanup%20for%20motion%20capture%20editing.pdf), even though that paper is not about this project's full dynamic problem.

### 11.2 Contact Changes Must Be Legible

When contact is created or broken, the event should be readable.

For example:

* take-off should correspond to real support loss
* landing should correspond to real support acquisition
* recovery stepping should create identifiable new support

### 11.3 Contact Integrity Has Higher Priority Than Cosmetic Pose Matching

If the system must choose between:

* preserving exact stylistic pose matching
* preserving support integrity

it should prefer support integrity.

This is especially important once the system later has to work on:

* slopes
* uneven surfaces
* rough terrain
* discontinuous footholds

---

## 12. Terrain-General Transition Requirement

The transition logic must not be written in a way that only makes sense on flat ground.

Even if the first implementation uses simple flat terrain, the conceptual transition design should already remain valid for:

* sloped support
* uneven support heights
* locally rough contact geometry
* discontinuous footholds

This means transitions should be described in terms of:

* support validity
* support exchange
* reachable footholds
* local support geometry
* groundedness

and not only in terms of:

* `ground_y = 0`
* horizontal floor assumptions
* symmetric flat-ground gait timing

This requirement is consistent with the terrain robustness demonstrated in [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf) and with the incline-sensitive reduced models discussed in [The Spring-Mass Model and Other Reductionist Models](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Spring-Mass%20Model%20and%20Other%20Reductionist%20Models.pdf).

---

## 13. Main Transition Failure Modes to Avoid

The following design errors should be explicitly avoided:

* snapping directly between procedural poses without respecting state continuity
* entering physical mode from a reset state instead of the actual current state
* returning from physical mode by forcing a canonical pose too early
* smoothing away legitimate physical impacts until the motion looks weightless
* preserving pose style while breaking support integrity
* hard-coding all transitions around flat-ground assumptions
* blending cyclic motions without respecting gait phase

These are not implementation details.
They are conceptual transition failures.

---

## 14. What Is Deferred to Later Documents

This document does not yet define:

* exact transition thresholds
* exact formulas for blending weights
* exact timing rules
* exact procedural phase inference method
* exact contact solver behavior
* exact Box2D integration details
* full validation tests for transition quality

These belong to later technical documents.

---

## 15. Summary of Main Takeaways

1. Continuity in this project is multi-layered: CM, support, contact, articulated pose, and visual continuity all matter.
2. Not every sharp change is bad; physically meaningful impacts and support changes may introduce legitimate discontinuities.
3. Procedural-to-procedural transitions should prefer phase-aware blending and compatible CM evolution.
4. Procedural-to-physical transitions should preserve the current real state, not reset into a pre-authored reaction.
5. Physical-to-procedural re-entry should happen through recovery, state capture, and re-synchronization, not snap.
6. Contact integrity is a first-class constraint, especially for future terrain complexity.
7. The transition architecture must remain valid for rough, sloped, uneven, and discontinuous terrain even if V1 begins on flat ground.
