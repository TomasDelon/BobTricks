# Control Architecture

## Implementation Alignment Note

This document remains valid as a **conceptual control-layer reference**.

For midpoint coding decisions, the implementation-facing documents take precedence:

- [`../implementation/00_Implementation_Freeze.md`](../implementation/00_Implementation_Freeze.md)
- [`../implementation/05_Midpoint_Procedural_Locomotion_Spec.md`](../implementation/05_Midpoint_Procedural_Locomotion_Spec.md)
- [`../implementation/06_Design_To_Implementation_Mapping.md`](../implementation/06_Design_To_Implementation_Mapping.md)

Important midpoint interpretation:

- midpoint is procedural-only
- walking remains pendular
- running remains spring-mass-like
- the future physical branch is architecturally reserved but inactive

---

## 1. Role of This Document

This document defines the **logical control architecture** of the project.

The previous documents established:

* the overall project vision
* the internal stickman model
* the conceptual locomotion models of the CM
* the hybrid regimes of the character system

What is still needed is a clear explanation of how the system is internally organized so that these concepts can actually work together.

This document therefore answers the following questions:

* Which control layers exist inside the system?
* What is the role of each layer?
* What information does each layer receive?
* What information does each layer produce?
* How do procedural control, support reasoning, physical response, and reconstruction interact?

This document remains **conceptual**.
It does not yet define:

* software classes
* exact APIs
* update loop implementation details
* exact torque formulas

Those elements belong to later design documents.

---

## 2. Why a Layered Control Architecture Is Needed

The project combines:

* nominal locomotion
* perturbation response
* recovery
* falling
* grounded stabilization
* get-up

It also combines:

* CM-based global reasoning
* support and contact logic
* articulated reconstruction
* physically reactive behavior

Because of this, a single monolithic controller would be hard to understand, hard to extend, and hard to keep compatible with future terrain complexity.

The system therefore needs a **layered architecture** in which different kinds of reasoning remain separate:

* high-level intention
* regime selection
* CM-level motion organization
* support and contact reasoning
* articulated target generation
* physical target tracking or physical reaction

This separation is consistent with the spirit of [SIMBICON](../references/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), where locomotion control is organized through a control structure rather than a single undifferentiated behavior, and with [Animation Bootcamp](../references/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf), where procedural motion, inverse kinematics, secondary physics, and ragdoll-like reactions are treated as cooperating layers rather than as a single tool.

---

## 3. Global Control Flow

At the highest level, information should flow through the system in the following order:

1. **intent is defined**
2. **the active hybrid regime is selected**
3. **the CM behavior is organized**
4. **support and contact are evaluated**
5. **foot placement decisions are made if needed**
6. **articulated body targets are reconstructed**
7. **physical tracking or physical reaction is applied**
8. **the resulting body state becomes the current character state**

This does not mean every layer always has equal importance.
Depending on the regime, some layers become dominant and others become secondary.

---

## 4. Main Control Layers

## 4.1 Intent Layer

### Role

The Intent Layer defines **what the character is trying to do**.

### Typical outputs

It may produce high-level requests such as:

* stand
* crouch
* walk
* run
* jump
* recover
* get up

### Inputs

Conceptually, it can receive:

* user intention
* scenario logic
* scripted requests
* internal state constraints

### Outputs

It outputs a **high-level locomotion objective** rather than a pose or torque.

### Important limitation

This layer should not decide the detailed body motion directly.
Its role is to express intent, not to solve contact, support, or physics.

---

## 4.2 Regime Manager

### Role

The Regime Manager decides **which hybrid regime is currently active**.

### Managed regimes

It selects among:

* nominal procedural
* physical reactive
* recovery
* falling
* grounded stabilization
* procedural get-up

### Inputs

It conceptually receives:

* current intent
* current CM state
* support state
* perturbation information
* groundedness
* recoverability status

### Outputs

It outputs:

* the active regime
* transition permissions or restrictions
* control priorities for lower layers

### Why this layer is necessary

Without a Regime Manager, the project would have no explicit place where it decides:

* whether the perturbation is still recoverable
* whether the body should remain procedural
* whether it should enter falling
* whether it may return to get-up and then to nominal motion

---

## 4.3 CM Controller

### Role

The CM Controller organizes the **global motion of the body as a whole**.

It is the main layer responsible for turning a locomotion objective into a meaningful CM evolution.

### Inputs

It conceptually receives:

* current locomotion intent
* active regime
* current CM state
* current support context

### Outputs

It may output:

* target CM position tendencies
* target velocity tendencies
* target acceleration tendencies
* target operating height
* regime-specific motion objectives

### Regime dependence

Its behavior depends strongly on the current locomotion mode:

* standing -> postural regulation
* walking -> pendular transport and redirection
* running -> spring-mass-like behavior
* crouching -> lowered operating point
* recovery -> recapture of controllable CM state

### Important limitation

The CM Controller does not decide alone whether a motion is physically feasible.
That is why it must remain coupled to support and reconstruction layers.

---

## 4.4 Support and Contact Reasoning Layer

### Role

The Support and Contact Reasoning Layer interprets the relation between the body and the environment.

It answers questions such as:

* what support is currently active?
* is the current support still valid?
* is the CM capturable?
* is a new support needed?
* is the body grounded?

### Inputs

It conceptually receives:

* current body state
* current CM state
* contact information
* terrain or support-surface information

### Outputs

It may output:

* support status
* grounded status
* support validity
* capturability assessment
* need for support change

### Why this layer is central

The project is not only CM-driven.
It is also support-constrained.

This layer is therefore essential because it connects:

* locomotion
* balance
* terrain
* recovery

### Terrain-general requirement

This layer must be designed in a way that later remains valid for:

* rough terrain
* sloped terrain
* uneven terrain
* discontinuous support geometry

This requirement is strongly motivated by [The Spring-Mass Model and Other Reductionist Models](../references/papers/The%20Spring-Mass%20Model%20and%20Other%20Reductionist%20Models.pdf), which explicitly extends reduced locomotion models to inclines, and by [SIMBICON](../references/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), which demonstrates robustness to slopes, steps, and terrain variation.

---

## 4.5 Foot Placement Planner

### Role

The Foot Placement Planner decides **where future support should be created** when stepping is required.

### Inputs

It conceptually receives:

* current CM state
* active regime
* support status
* current body configuration
* support-surface information

### Outputs

It may output:

* next support target
* nominal step target
* recovery step target
* multi-step recovery plan at a high conceptual level

### Why this layer matters

This layer is crucial because recovery may require:

* no step
* one recovery step
* several recovery steps

It is also the layer most directly responsible for future extension to more complex terrain.

---

## 4.6 Articulated Reconstruction Layer

### Role

The Articulated Reconstruction Layer turns global targets into body configuration targets.

It is the layer that transforms:

* CM goals
* support targets
* regime-specific constraints

into:

* articulated body targets

### Inputs

It conceptually receives:

* target CM behavior
* current support or foot placement targets
* active regime
* current stickman geometry

### Outputs

It may output:

* target node positions
* target segment configuration
* target articulated pose

### Why it is needed

The CM does not directly give a full body pose.
The body must be reconstructed in a way that remains:

* geometrically plausible
* support-consistent
* visually coherent

This layer is conceptually related to articulated figure reconstruction and inverse-kinematics reasoning such as that discussed in [Inverse Kinematics and Geometric Constraints for Articulated Figure Manipulation](../references/papers/Inverse%20Kinematics%20and%20Geometric%20Constraints%20for%20Articulated%20Figure%20Manipulation.pdf).

---

## 4.7 Physical Tracking Layer

### Role

The Physical Tracking Layer is responsible for the interaction between:

* procedural targets
* physical body behavior

This is the layer that later supports:

* active-ragdoll-like tracking
* pose-following under physics
* physically controlled recovery

### Inputs

It conceptually receives:

* target articulated pose
* current physical body state
* active regime

### Outputs

It may output:

* tracking torques
* actuation objectives
* physical guidance commands

### Important clarification

This layer is not the same as the pure procedural generator.
Its role is not to define what the motion should be.
Its role is to help the physical body follow or partially follow the desired motion when appropriate.

---

## 4.8 Failure Handling Layer

### Role

The Failure Handling Layer manages situations where recovery is no longer plausible.

### Inputs

It conceptually receives:

* recoverability failure
* support failure
* falling state indicators
* grounded state indicators

### Outputs

It may output:

* falling transition decisions
* relaxation of nominal control priorities
* entry into grounded stabilization
* permission to enter get-up later

### Why it is necessary

Without explicit failure handling, the system would tend to:

* keep trying impossible recoveries
* snap into unrealistic postures
* blur the distinction between recoverable and unrecoverable events

This layer exists to make failure a structured part of the architecture.

---

## 5. Information Exchange Between Layers

The layers do not operate independently.
They exchange specific categories of information.

### 5.1 Typical Information Flow

At a high level:

* **Intent Layer -> Regime Manager**
  provides desired behavior

* **Regime Manager -> CM Controller**
  provides active regime and control interpretation

* **CM Controller -> Support and Contact Reasoning**
  provides global motion tendencies

* **Support and Contact Reasoning -> Foot Placement Planner**
  provides support validity and need for new support

* **Foot Placement Planner -> Articulated Reconstruction**
  provides support targets

* **CM Controller -> Articulated Reconstruction**
  provides global body transport targets

* **Articulated Reconstruction -> Physical Tracking**
  provides target body configuration

* **Failure Handling Layer -> Regime Manager**
  provides failure-driven regime transition information

### 5.2 Why This Matters

The goal of this section is not to define APIs yet.
It is to make clear that:

* each layer has a distinct role
* each layer depends on specific upstream information
* the architecture is not a bag of disconnected modules

---

## 6. Regime-Dependent Reweighting of the Architecture

The same layers do not have the same importance in every regime.

### 6.1 In Nominal Procedural Motion

The dominant layers are:

* Intent Layer
* Regime Manager
* CM Controller
* Articulated Reconstruction

Physical Tracking may still be present, but mainly as support.

### 6.2 In Physical Reactive Motion

The dominant layers are:

* Regime Manager
* Support and Contact Reasoning
* Physical Tracking

The CM Controller becomes less prescriptive and more adaptive.

### 6.3 In Recovery

The dominant layers are:

* Regime Manager
* Support and Contact Reasoning
* Foot Placement Planner
* CM recapture logic
* Physical Tracking

### 6.4 In Falling

The dominant layers are:

* Failure Handling
* Support and Contact Reasoning
* Physical Tracking

Nominal locomotion layers are no longer dominant.

### 6.5 In Get-Up

The dominant layers are:

* Regime Manager
* Articulated Reconstruction
* Physical Tracking

The system progressively shifts back toward procedural organization.

### 6.6 Why This Is Important

This is one of the most important architectural principles of the project:

the architecture is stable, but the **weight of its layers changes with the regime**.

This avoids needing a completely different software system for every state of the character.

---

## 7. Terrain-General Requirement

The control architecture must not be written as if locomotion only existed on a single flat horizontal line.

### 7.1 Minimum Architectural Requirement

Even if V1 begins with simple ground, the architecture must already support the conceptual possibility of:

* local support geometry
* changing support normals
* irregular reachable footholds
* support-dependent recovery logic

### 7.2 Where This Requirement Lives

This requirement primarily affects:

* Support and Contact Reasoning
* Foot Placement Planner
* Regime Manager

because those are the layers most directly affected by support geometry.

### 7.3 Why It Matters

If these layers are designed only for flat ground, later terrain extension may force a redesign of the whole control system.
This is exactly what the architecture should avoid.

---

## 8. What This Architecture Must Make Possible

The architecture described here should make the following capabilities possible:

* stable nominal procedural locomotion
* CM-organized whole-body motion
* support-aware walking and running
* perturbation-triggered physical reaction
* recovery without stepping
* recovery through one or several steps
* transition into falling when recovery is no longer plausible
* grounded stabilization after impact
* re-entry into procedural get-up
* later extension to more complex terrain

This is the practical meaning of the architecture.

---

## 9. What Is Deferred to Later Documents

This document does **not** yet define:

* exact perturbation metrics
* exact switching thresholds
* exact recovery formulas
* exact joint PD equations
* exact software classes
* exact update loop order
* exact rendering layer implementation

Those topics will be addressed later.

---

## 10. Summary of Main Takeaways

1. The project requires a layered control architecture because locomotion, perturbation, support, reconstruction, and physical reaction should not all be merged into one undifferentiated controller.
2. The main conceptual layers are: Intent, Regime Manager, CM Controller, Support and Contact Reasoning, Foot Placement Planner, Articulated Reconstruction, Physical Tracking, and Failure Handling.
3. These layers exchange structured information rather than raw uncontrolled behavior.
4. The same layers remain present across regimes, but their relative importance changes depending on the active regime.
5. The architecture must remain extensible to future rough, sloped, uneven, and discontinuous terrain.

This document therefore establishes the **logical control-architecture layer** of the project.
The next document should formalize the physical pose tracking logic in more detail.
