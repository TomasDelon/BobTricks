# Project Vision

## 1. Purpose

The purpose of this project is to create a **2D stickman animation system** whose motion is generated primarily through **procedural control** rather than through fixed animation clips.

The character is intended to follow a **center-of-mass-first (`CM-first`) philosophy** in nominal locomotion, while also remaining **hybrid**: procedural when motion is under control, and physically reactive when perturbation or impact becomes dominant.

The scientific ambition is not full biomechanical realism, but motion grounded in simplified principles inspired by system-level CoM analysis, pendular walking, spring-mass running, and robust push-recovery control, as discussed in [The Motion of Body Center of Mass During Walking](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf), [Kuo 2007](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Kuo2007-Six-Determinants-Gait-Inverted-Pendulum.pdf), [The Spring-Mass Model and Other Reductionist Models](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Spring-Mass%20Model%20and%20Other%20Reductionist%20Models.pdf), and [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf).

The result is intended to be both a locomotion system and a visual character system: internally, the body remains a simplified articulated model; externally, it should appear as a **continuous line-like stickman** rendered in **SDL**.

The project is also constrained by a mandatory **Model-View-Controller (MVC)** architecture, so locomotion state, rendering, and orchestration must remain clearly separated from the beginning.

The role of this document is to establish the global vision of the project before moving on to detailed model design, hybrid control regimes, software architecture, and implementation planning.

## 2. Core Idea

The core idea of the project is to animate the character through a **layered hybrid process** rather than through a single universal animation method.

In its nominal regime, the character is driven procedurally.
At that stage, the global motion of the body is organized from the top down:

1. a locomotion mode defines the current behavioral intent
2. the **CM** evolves according to that mode
3. support logic and foot placement are derived from the CM state
4. the articulated stickman is reconstructed to remain visually and mechanically plausible

This means that nominal motion does not begin from local joint angles and then derive the body behavior afterward.
Instead, it begins from a global locomotion description and lets the articulated body follow it.
This project therefore adopts the same general system-level viewpoint highlighted in [The Motion of Body Center of Mass During Walking](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf), where locomotion is understood primarily as transport of the body as a whole rather than only as a sequence of local limb rotations.

However, the project does not assume that procedural control is sufficient in all situations.
When the character is strongly perturbed by the environment, for example by a push, impact, or destabilizing contact, the system may transition into a **physically reactive regime**.
In that regime, body motion is no longer prescribed only by the nominal CM controller.
Instead, the articulated body responds through physical dynamics, contact, and control torques, closer in spirit to physically simulated character control and active-ragdoll-like strategies, as seen in [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), [Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf), and [Advanced Character Physics](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Advanced%20Character%20Physics.pdf).

From there, two broad outcomes are possible:

* if the perturbation remains recoverable, the character attempts to regain a controllable state and eventually re-synchronize with the procedural locomotion controller
* if the perturbation is too strong, the character transitions into falling and landing behavior instead of forcing an implausible recovery

After falling, the character should not remain a passive ragdoll indefinitely.
Once a valid grounded configuration is reached, the system should be able to generate a new procedural sequence allowing the character to reorganize its body, return to a stable posture, and eventually stand up again.

In compact form, the global logic of the project can be described as two possible branches after perturbation:

`procedural locomotion -> perturbation -> physical reaction -> recoverable -> recovery -> resynchronization -> procedural locomotion`

or

`procedural locomotion -> perturbation -> physical reaction -> unrecoverable -> fall -> grounded stabilization -> procedural get-up -> procedural locomotion`

This sequence is the conceptual backbone of the entire project.
It defines the intended relation between locomotion modeling, physical interaction, recovery behavior, and rendering, and it is the reason why the project must be designed from the beginning as a hybrid character system rather than as a pure animation player or a pure physics ragdoll.

## 3. Scientific and Conceptual Inspiration

The project is not intended to imitate the full complexity of human biomechanics.
Instead, it is inspired by a set of simplified but scientifically meaningful principles that can organize character motion at a global level.

### 3.1 Locomotion as Transport of the Body as a Whole

A central idea behind the project is that locomotion should not be understood only as a sequence of local joint rotations.
It should also be understood as the controlled transport of the body as a whole through space.
This system-level viewpoint is strongly emphasized in [The Motion of Body Center of Mass During Walking](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf), which argues that the CoM provides a compact and meaningful description of locomotor behavior.
The project adopts that perspective directly by treating the CM as a primary locomotion variable during nominal motion generation.

### 3.2 Reduced Models of Walking and Running

The project is also inspired by the idea that different gaits should not be explained by a single identical motion model.
Instead, different locomotion regimes may require different reduced descriptions of the CM dynamics.

For walking, the main conceptual reference is the pendular interpretation of gait discussed in [Kuo 2007](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Kuo2007-Six-Determinants-Gait-Inverted-Pendulum.pdf), where the body passes over the stance leg and must be redirected from one support phase to the next.

For running and bouncing locomotion, the main conceptual reference is the spring-mass family of models discussed in [The Spring-Mass Model and Other Reductionist Models](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Spring-Mass%20Model%20and%20Other%20Reductionist%20Models.pdf), as well as classical running biomechanics references such as [Biomechanics of Running Gait](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Biomechanics%20of%20Running%20Gait.pdf) and [Mechanical Work and Efficiency in Level Walking and Running](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Mechanical%20work%20and%20efficiency%20in%20level%20walking%20and%20running.pdf).

These models are used not as full body descriptions, but as compact principles for shaping plausible CM motion.

### 3.3 Balance, Support, and Recovery

Another key inspiration is the idea that balance is maintained not only by posture, but also by continuous adaptation of support.
In particular, locomotion control can be made robust by adjusting future support placement as a function of CM state.
This idea is central in [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), where balance and locomotion are closely linked through feedback based on center-of-mass position and velocity.
The project does not aim to copy SIMBICON directly, but it adopts the same general lesson: robust locomotion should connect CM evolution, support decisions, and perturbation response.

### 3.4 Hybrid Character Animation

From the animation side, the project is inspired by the observation that believable character motion often benefits from combining multiple methods instead of relying on one single technique.

[Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf) is especially relevant here because it presents procedural animation, inverse kinematics, springs, secondary physics, and ragdoll-like ideas as components of a unified animation strategy.
Likewise, [Advanced Character Physics](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Advanced%20Character%20Physics.pdf) shows that articulated bodies and constraints can be handled in a stable, interactive way when physical plausibility is prioritized over exact realism.
Together, they support the hybrid direction of the project: procedural control in nominal behavior, physical simulation under strong perturbation, and controlled re-entry into animation after recovery or grounded stabilization.

### 3.5 Articulation and Contact Credibility

Although the project is centered on the CM, it cannot ignore articulated plausibility and contact quality.
The literature on articulated figure manipulation and contact cleanup shows that if reconstruction is poor, the illusion breaks even if the global motion idea is correct.
This is why inverse kinematics and contact consistency remain important secondary layers, as illustrated by [Inverse Kinematics and Geometric Constraints for Articulated Figure Manipulation](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Inverse%20Kinematics%20and%20Geometric%20Constraints%20for%20Articulated%20Figure%20Manipulation.pdf) and [Footskate Cleanup for Motion Capture Editing](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Footskate%20cleanup%20for%20motion%20capture%20editing.pdf).
For this reason, articulated reconstruction is treated not as the source of locomotion, but as a necessary layer for preserving visual credibility and support consistency.

### 3.6 Position of the Project

Taken together, these references suggest a coherent design direction:

* model locomotion globally through the CM
* distinguish gait regimes conceptually rather than forcing one motion law for all cases
* connect balance to support adaptation
* combine procedural and physical methods instead of choosing only one
* preserve articulated and contact plausibility in the final body motion

The project should therefore be understood as a **biomechanically inspired hybrid character system**, not as a strict robotics model and not as a purely stylistic animation system.

## 4. What CM-First Means Here

The expression **CM-first** can easily be misunderstood, so it must be defined precisely for this project.

In a classical articulated animation pipeline, one often starts from a body pose and computes the center of mass afterward.
That is **not** the main logic adopted here during nominal locomotion.
Instead, the project treats the **CM as a primary global locomotion state** and lets the rest of the body be organized around it.

This means that, in nominal procedural behavior:

* the CM is not introduced merely as a passive measurement of the body
* the CM is used as a compact driver of global motion
* support logic, foot placement, and articulated reconstruction are derived from the evolving CM state

In other words, the body does not define the CM first. Rather, the intended locomotor behavior defines an admissible CM evolution, and the body is then reconstructed so that the resulting pose remains plausible.

However, **CM-first does not mean arbitrary CM motion**.
The CM is not allowed to move freely without structure.
Even during procedural control, its evolution must remain constrained by:

* support conditions
* gait-dependent dynamics
* balance requirements
* reachable body geometry
* perturbation response logic

The CM is therefore not treated as a purely geometric curve drawn in space, but as a controlled locomotion state that must remain compatible with a body that can actually support and express that motion.

Another important clarification is that **CM-first applies primarily to the nominal procedural regime**.
It does not imply that the CM is always imposed in every phase of the system.
When the character transitions into a physically reactive regime, especially under strong perturbation, the body is no longer driven only by nominal procedural CM targets.
At that point, the articulated physical state evolves under dynamics, contact, and control torques, and the resulting CM becomes increasingly **emergent** rather than directly prescribed.

The project therefore distinguishes two interpretations of the CM:

* in **procedural nominal motion**, the CM is mainly a **controlled locomotion variable**
* in **physical reactive motion**, the CM is mainly an **emergent dynamic quantity**

This distinction is central because it allows the system to remain simple and expressive during normal movement while still becoming physically credible under perturbation, recovery attempts, and falls.

Finally, CM-first should also be understood as a modeling choice rather than as a claim of anatomical completeness.
The project does not try to reproduce the full human body and then measure its true biomechanical center of mass at all times.
Instead, it uses the CM as a meaningful global abstraction, consistent with the system-level viewpoint emphasized in [The Motion of Body Center of Mass During Walking](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf), in order to organize believable locomotion in a simplified stickman model.

## 5. Hybrid Animation Philosophy

The project does not aim to choose between **procedural animation** and **physics simulation** as if they were mutually exclusive alternatives.
Its central design choice is instead to combine them in a structured way, giving each one a different role depending on the state of the character.

### 5.1 Why a Hybrid Strategy Is Needed

A purely procedural character is easier to direct, easier to keep visually clean, and well suited to nominal locomotion.
However, if strong contact, impact, or perturbation occurs, purely procedural motion can quickly become implausible unless the controller is unrealistically rigid.

Conversely, a purely physical character reacts naturally to forces and impacts, but it is much harder to keep expressive, intentional, and stable during normal locomotion.

The project therefore adopts a hybrid philosophy:

* use **procedural control** where intention, readability, and locomotion organization matter most
* use **physical response** where impact, disturbance, and dynamic reaction matter most
* allow transitions between these regimes instead of forcing one regime to solve every situation

### 5.2 Nominal Procedural Regime

During nominal behavior, the character is driven procedurally.
This is the regime used for most stable and intentional actions such as:

* standing
* crouching
* walking
* running
* jumping under expected conditions

In this regime, locomotion remains organized by the CM-first approach.
The purpose of procedural control here is to produce motion that is:

* smooth
* readable
* controllable
* biomechanically inspired

### 5.3 Physical Reactive Regime

When the character experiences a significant perturbation, the project may transition into a physically reactive regime.
This can happen after:

* pushes
* collisions
* destabilizing landings
* unexpected support changes

In that regime, the articulated body is no longer interpreted only through nominal locomotion rules.
Instead, it is allowed to react through physical dynamics, contacts, and control torques.

The goal of this regime is not to abandon all control.
It is to let the body react credibly while still preserving the possibility of recovery if that recovery remains dynamically plausible.

### 5.4 Recovery Regime

If a perturbation is classified as recoverable, the character should not instantly snap back to its nominal procedural pose.
Instead, it should pass through a recovery phase.

The recovery regime exists to:

* regain a controllable support situation
* reduce excessive momentum
* restore a stable body organization
* reconnect physical state with procedural intent

This means recovery is not only a visual blend.
It is a transitional control phase in which the system tries to recapture balance and move back toward a valid locomotion state.

### 5.5 Falling Regime

If the perturbation is not recoverable, the project should not force a false recovery.
In that case, the correct behavior is to transition into falling.

The falling regime should be treated as a valid system state, not as an error condition.
Its role is to let the body:

* lose nominal balance control
* react physically to gravity and contact
* reach the ground in a believable way
* preserve as much plausibility and self-protection as possible

This is important because a convincing fall often contributes more to perceived realism than an implausible attempt to remain upright.

### 5.6 Grounded Stabilization and Get-Up

After a fall, the character should eventually reach a grounded state in which large-scale instability has dissipated.
This grounded phase is important because it creates the bridge back toward intentional animation.

From there, the system should be able to:

* identify a valid grounded body configuration
* select or generate a plausible get-up strategy
* progressively return from physical reaction to procedural control

This means that getting up is not an isolated animation clip in the conceptual sense.
It is the final stage of a hybrid cycle that begins with nominal motion, passes through disturbance and possibly falling, and ends with re-entry into intentional locomotion.

## 6. Target Behaviors

The project is intended to produce a character capable of exhibiting a coherent set of nominal, reactive, and recovery behaviors.
These behaviors are not independent animation clips.
They are different expressions of a single hybrid locomotion system.

### 6.1 Nominal Behaviors

The first family of behaviors corresponds to stable and intentional actions performed under normal conditions.
These include:

* standing still
* maintaining postural balance with small natural oscillations
* crouching
* standing up from a crouched configuration
* walking
* running
* jumping
* landing under expected conditions

These behaviors are expected to remain primarily procedural and CM-driven.

### 6.2 Reactive Behaviors

The second family of behaviors corresponds to the character’s response to external disturbance or destabilizing interaction.
These include:

* reacting to a push
* reacting to impact or collision
* compensating for sudden destabilization
* modifying posture under perturbation
* attempting support adaptation when balance is threatened

These behaviors are expected to involve the physically reactive regime more strongly than nominal locomotion.

### 6.3 Recovery Behaviors

The third family of behaviors corresponds to situations in which the character has been disturbed but is still capable of regaining control.
These include:

* regaining balance after a perturbation
* re-establishing a controllable support state
* reducing residual instability after a hard landing
* returning from physical reaction toward nominal locomotion

These behaviors are especially important because they define the quality of the hybrid transition between physical response and procedural control.

### 6.4 Failure and Post-Failure Behaviors

The fourth family of behaviors corresponds to situations in which recovery is no longer possible.
These include:

* falling
* contacting the ground in a believable way
* stabilizing on the ground after impact
* reorganizing the body into a recoverable grounded configuration
* getting up from the floor

These behaviors ensure that the system remains coherent even when nominal balance is lost.

### 6.5 Behavioral Objective

Taken together, the target behavior set is meant to produce a character that:

* moves intentionally under normal conditions
* reacts credibly to the environment
* attempts recovery when possible
* accepts falling when necessary
* can eventually return to intentional behavior after destabilization

This behavioral continuity is one of the main reasons for adopting a hybrid architecture rather than a collection of isolated animation states.

## 7. Visual Goal

The project is not only about generating plausible motion.
It is also about presenting that motion through a clear and intentional visual language.

The internal body model of the character is deliberately simplified.
It is built from a small articulated set of nodes and segments that make locomotion control, balance logic, and physical switching easier to reason about.
However, the final rendered result should **not** look like a rigid debugging skeleton made of disconnected straight bars.

### 7.1 A Continuous-Line Stickman

The visual goal is for the character to be perceived as a **single continuous stickman figure**.
Even if the underlying simulation uses discrete nodes and articulated segments, the final representation should appear visually unified and fluid.
The visible body should therefore be drawn through a continuous curve-based representation rather than through a collection of isolated segment primitives.

### 7.2 Separation Between Internal Structure and Visual Appearance

An important design principle of the project is that the **simulation structure** and the **rendered form** do not need to be identical.

Internally:

* the character is a simplified articulated body
* the locomotion system reasons about nodes, supports, contacts, and control states

Visually:

* the character should appear as a coherent drawn figure
* the visible body should smooth over the discrete nature of the internal model
* the rendering should support the impression of continuity and expressive motion
This separation matters because the simplified body graph is chosen for control and clarity, not because it is already the ideal final visual shape.

### 7.3 Curves, Splines, and SDL Rendering

To reach this visual objective, the project must include a rendering layer capable of constructing continuous curves from the current stickman pose.
In practice, this implies the need for a dedicated module that can:

* receive the current body pose from the simulation state
* construct curve control points from the articulated stickman structure
* generate spline-like or curve-based visual geometry
* render the resulting continuous figure in **SDL**

This module is not secondary decoration.
It is part of the identity of the project, because the final character is expected to read visually as a continuous line-based figure rather than as a raw articulated skeleton.

### 7.4 Visual Coherence Under Motion and Perturbation

The visual representation should remain coherent not only during calm locomotion, but also during:

* perturbation
* recovery
* falling
* grounded stabilization
* getting up

This means that the rendering layer must tolerate large pose changes without visually breaking the body into disjoint parts.
The character should remain legible across the whole hybrid behavior range.

## 8. Software and Architecture Constraints

The project is not defined only by its locomotion and animation goals.
It is also constrained by explicit software and architectural requirements that must shape the design from the beginning.

### 8.1 Implementation Context

The system is intended to be implemented in **C++** and rendered through **SDL**.
For development-time inspection and debugging tools, the project may also rely on **Dear ImGui**.
For vectors, points, and linear-algebra operations, the project should use a dedicated mathematics library such as **Eigen** rather than ad hoc homemade utility types.
This means that the final solution must remain compatible with:

* real-time update constraints
* a custom rendering pipeline rather than a high-level game engine
* explicit control over simulation state, rendering state, and update order

This implementation context is important because it favors clear internal structure, explicit module boundaries, and a strong separation between simulation and visualization.

### 8.2 MVC Is Mandatory

The project must explicitly follow a **Model-View-Controller (MVC)** architecture.
This is not an optional stylistic preference.
It is a design constraint of the project.

As a consequence, the project should be organized around three clearly distinct responsibilities:

* the **Model**, which stores and updates locomotion state, body state, physical state, support state, and control targets
* the **View**, which renders the stickman, debug information, and curve-based visual output in SDL
* the **Controller**, which interprets inputs, triggers perturbations, selects modes, and orchestrates the interaction between user intent and system behavior

### 8.3 Separation of Responsibilities and Rendering Module

Because the final character must appear as a continuous line rather than as disconnected rigid segments, the software architecture must include a dedicated rendering component capable of producing such a representation.

At a conceptual level, this component should:

* receive the current simulated pose from the Model
* convert the simplified articulated structure into a continuous visual form
* generate and render curve-based geometry in SDL

Under MVC, this requirement belongs to the **View**, while the **Model** remains responsible for locomotion state, body state, support state, and physical state.
This requirement is important enough that it should be treated as a first-class architectural concern, not as a cosmetic post-process added at the end.

### 8.4 Control, Physics, and Rendering Must Remain Distinct

The project also requires a clean separation between three different kinds of logic:

* **locomotion control logic**
* **physical reaction logic**
* **visual rendering logic**

If these are mixed too early, the project becomes much harder to reason about.
For that reason, the architecture should keep distinct:

* what the character is trying to do
* what physically happens to the character
* how the current state is drawn on screen

### 8.5 Mistakes to Avoid for Future Terrain Extension

Even if the first implementation focuses on simple ground conditions, the architecture should avoid design choices that would make future terrain adaptation unnecessarily difficult.

The following mistakes should be avoided from the beginning:

* hard-coding the assumption that the ground is always a single horizontal line
* hard-coding a constant world-up contact normal everywhere in locomotion logic
* assuming that every foot contact occurs at a fixed global height
* mixing terrain interpretation directly into locomotion state logic instead of isolating it behind dedicated support or terrain queries
* coupling foot placement rules to a flat-ground-only representation
* treating support as a purely visual concept rather than as a geometric and physical one
* embedding SDL rendering assumptions inside terrain, locomotion, or support reasoning
* designing recovery logic that only works when support geometry is trivial

The intended strategy is therefore:

* keep the **first implementation simple**
* but keep the **architecture extensible**

In practical terms, flat ground may be enough for V1 behavior, but the design should already think in terms of:

* support surfaces
* contact validity
* local ground geometry
* reachable support points
* support-dependent balance conditions

This will make it possible to extend the system later toward uneven, sloped, or discontinuous terrain without having to redesign the entire project.

## 9. Design Principles

The following principles should guide design decisions throughout the project.
They are not implementation details.
They are high-level rules intended to keep the project coherent as it grows in scope and complexity.

### 9.1 Global Motion Before Local Detail

The project prioritizes the global organization of motion before local articulation detail.
This is the main reason for the CM-first approach.
If local joint motion looks acceptable but the global body transport feels implausible, the result should still be considered unsatisfactory.

### 9.2 Biomechanical Plausibility Over Arbitrary Animation

The goal is not perfect biomechanical realism, but the motion should remain consistent with simplified scientific principles whenever possible.
When choosing between a purely arbitrary movement rule and one that preserves system-level plausibility, the latter should be preferred.

### 9.3 Procedural Where Controlled, Physical Where Necessary

Nominal locomotion should remain procedural and organized.
Physical simulation should become dominant when the body is strongly perturbed or when contact dynamics can no longer be represented credibly through nominal control alone.

### 9.4 Recovery Before Forced Stability

If the character is disturbed, the system should first attempt believable recovery rather than immediately snapping back to a stable animation state.
Likewise, if recovery is no longer plausible, the system should accept falling rather than fake an impossible stabilization.

### 9.5 Continuity Matters

The project should favor temporal continuity in CM motion, support changes, and articulated behavior whenever the situation remains under control.
Abrupt transitions are acceptable when required by impact or loss of balance, but not as a substitute for missing control logic.

### 9.6 Internal Simplicity, External Coherence

The internal body model may remain simple if the global behavior is clear and the final visible result is coherent.
The project does not require anatomical complexity for its own sake.
It requires enough internal structure to support believable motion and enough external smoothing to support a strong visual identity.

### 9.7 Clear Separation of Responsibilities

The project should preserve a strong separation between:

* locomotion logic
* physical response logic
* visual rendering logic

This principle is especially important because MVC is mandatory and because the final rendered shape is intentionally different from the internal articulated model.

### 9.8 Failure Is Part of the System

Loss of balance, falling, and grounded recovery should be treated as legitimate behaviors rather than as edge cases outside the design.
A character system that only works when everything goes well does not satisfy the central goal of this project.

## 10. First Version Scope and Deliverable

The project vision is broad, but the first implementation must remain deliberately constrained.
The purpose of the first version is not to solve every possible locomotion and animation problem.
It is to build a coherent and testable core system that demonstrates the main hybrid principles of the project.

### 10.1 Included in the First Version

The first version should include at least the following capabilities:

* a simplified 2D stickman body model
* a procedural nominal locomotion regime organized around the CM
* basic support logic and articulated reconstruction
* physically reactive behavior under perturbation
* distinction between recoverable and unrecoverable perturbations
* falling behavior when recovery fails
* grounded stabilization after a fall
* a procedural get-up sequence or get-up strategy from the ground
* rendering of the character as a continuous line-like figure in SDL
* a software architecture consistent with MVC

The first version should also provide enough internal observability to support development and validation, for example through debug visualization of:

* nodes
* CM position
* support contacts
* current control regime

### 10.2 Behaviors That Should Be Present in V1

In behavioral terms, V1 should aim to demonstrate:

* standing
* crouching
* walking
* running
* jumping in a basic form
* landing in a basic form
* perturbation response
* recovery attempt
* falling
* getting up

The emphasis should be on coherence of the hybrid cycle rather than on an exhaustive catalog of actions.

### 10.3 Not Required Yet in the First Version

The first version does **not** need to solve all of the following:

* full anatomical realism
* detailed muscle modeling
* robust locomotion over uneven, sloped, or discontinuous terrain
* complex hand interaction
* complex environmental manipulation
* highly varied get-up styles
* motion capture integration
* machine learning or data-driven controllers
* advanced artistic styling beyond the continuous-line objective

These elements may become relevant later, but they should not define the success criteria of the first version.

### 10.4 Objective of V1

The real objective of the first version is to prove that the following central loop can be implemented coherently:

* intentional procedural motion
* transition into physical reaction under perturbation
* recovery if possible
* falling if not
* return to intentional motion afterward

If this loop works in a believable and architecturally clean way, then the first version will already validate the main design direction of the project.

### 10.5 Expected Deliverable

The expected deliverable is a coherent **interactive prototype** demonstrating the main architectural and behavioral ideas of the system.

At a minimum, it should provide:

* a 2D stickman implemented in **C++**
* real-time rendering in **SDL**
* a continuous line-like visual representation
* nominal procedural locomotion organized around the CM
* a physically reactive regime triggered by perturbation
* distinction between recoverable and unrecoverable disturbance
* recovery when possible
* falling and grounded stabilization when recovery fails
* procedural re-entry through a get-up process
* an overall software organization consistent with **MVC**

The prototype does not need to prove every advanced capability.
Its role is to validate the chosen hybrid direction and show that the main control loop can be implemented coherently.

## 12. Document Roadmap

This document defines the high-level vision of the project.
It should be followed by more focused documents that progressively refine the body model, locomotion logic, hybrid control structure, rendering logic, and software architecture.

The next design documents should include at least:

* [01_Stickman_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/01_Stickman_Model.md): body topology, node structure, geometry, and nominal CM references
* [02_CM_And_Locomotion_Models.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/02_CM_And_Locomotion_Models.md): CM state definition and locomotion regimes such as standing, walking, running, jumping, recovery, and falling
* [03_Hybrid_Control_Regimes.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/03_Hybrid_Control_Regimes.md): procedural mode, physical reactive mode, recovery mode, falling mode, and re-entry conditions
* [04_Control_Architecture.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/04_Control_Architecture.md): main control layers, support planning, reconstruction logic, and orchestration
* [05_Physical_Tracking.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/05_Physical_Tracking.md): PD-based pose tracking, active-ragdoll-like behavior, and controlled physical recovery
* [06_Transitions_And_Continuity.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/06_Transitions_And_Continuity.md): procedural-to-physical and physical-to-procedural transitions, temporal continuity, and re-synchronization
* [07_Rendering_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/07_Rendering_Model.md): mapping from articulated body state to curve-based rendering and continuous-line visual representation in SDL
* [08_Software_Architecture_MVC.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/08_Software_Architecture_MVC.md): formal MVC decomposition and software responsibilities
* [09_Physical_Body_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/09_Physical_Body_Model.md): mapping from the abstract stickman to Box2D bodies, joints, shapes, limits, foot/support geometry, and collision policy
* [10_Reconstruction_And_IK.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/10_Reconstruction_And_IK.md): reconstruction of articulated posture from CM state, support constraints, and nominal locomotion regime
* [11_Recovery_Metrics.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/11_Recovery_Metrics.md): operational definition of capturable, recoverable, and unrecoverable states for balance recovery and fall decisions
* [12_Update_Loop_And_Timing.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/12_Update_Loop_And_Timing.md): main update order, Box2D stepping policy, simulation timing, and render timing
* [13_Modules_And_Responsibilities.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/13_Modules_And_Responsibilities.md): module list, data responsibilities, and interactions
* [14_Assumptions_And_Open_Questions.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/14_Assumptions_And_Open_Questions.md): explicit assumptions, unresolved issues, and future design risks
* [15_Validation_Principles.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/15_Validation_Principles.md): how to evaluate physical and biomechanical consistency of the procedural and hybrid behaviors

This roadmap exists to ensure that the project moves from high-level intention to precise design in a controlled and traceable way, instead of collapsing too early into implementation details.
