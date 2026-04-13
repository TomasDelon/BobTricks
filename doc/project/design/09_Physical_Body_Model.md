# Physical Body Model

## 1. Role of This Document

This document defines the **physical body model** used by the project.

The previous documents established:

* the abstract stickman model
* the CM-centered locomotion logic
* the hybrid regime structure
* the control architecture
* the rendering model
* the MVC decomposition

What is still missing is the explicit description of the body that will exist inside the physical simulation.

This document therefore answers the following questions:

* What physical body exists in the project?
* How is the abstract stickman mapped to Box2D bodies and joints?
* What support geometry is used?
* What contacts matter?
* What collision policy is used?
* How does the project transition from procedural nominal motion into physical motion?

This document does **not** yet define:

* the full reconstruction and IK process
* the formal recovery metric
* the exact update loop
* the exact tuning of gains

Those topics belong to later documents.

---

## 2. Architectural Decision for V1

The first version of the project will **not** use an always-physical character architecture.

### 2.1 Chosen V1 Architecture

For V1, the character will be:

* primarily **procedural / kinematic** in the nominal regime
* **physically simulated in Box2D** during perturbation, falling, grounded stabilization, and related strongly reactive phases

This means that V1 uses a **hybrid switch-based architecture**:

* nominal locomotion is organized procedurally
* strong disturbance activates the physical body
* the physical regime handles reactive motion
* the system later attempts controlled re-entry to procedural behavior

### 2.2 Why This Architecture Was Chosen

This choice is made primarily for **project feasibility**.

With the available time budget, an always-dynamic active-ragdoll architecture would create major additional difficulty in:

* nominal gait stability
* foot contact jitter
* torque tuning
* contact/constraint conflicts inside Box2D
* keeping the nominal motion readable

For a longer project, these costs could be justified.
For V1, they represent too much technical risk.

### 2.3 Future Evolution

A future version of the project may evolve toward an **always-physical** architecture in which:

* the same articulated dynamic body exists at all times
* procedural locomotion provides targets continuously
* perturbation changes control authority rather than changing body nature

That evolution is recognized as conceptually strong, but it is postponed because V1 must remain achievable.

---

## 3. Why a Physical Body Model Is Still Necessary

Even though nominal locomotion is procedural in V1, the project still needs a real physical body model.

This is required for:

* pushes
* impacts
* collisions
* destabilizing landings
* falling
* grounded stabilization
* get-up preparation

This is consistent with the physically reactive character logic suggested by [Advanced Character Physics.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Advanced%20Character%20Physics.pdf), and with the push-recovery and reactive stepping literature such as [Humanoid Push Recovery.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Humanoid%20Push%20Recovery.pdf) and [Push Recovery Control for Force-Controlled Humanoid Robots.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Push%20Recovery%20Control%20for%20Force-Controlled%20Humanoid%20Robots.pdf).

The physical body is therefore not optional.
It is a required subsystem of the hybrid character.

---

## 4. Relation Between the Abstract Stickman and the Physical Body

The physical body is **derived from**, but not identical to, the abstract stickman model of [01_Stickman_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/01_Stickman_Model.md).

### 4.1 The Abstract Stickman Is a Structural Model

The abstract stickman defines:

* visible nodes
* topological structure
* nominal geometry
* CM reference logic

It is primarily a control-oriented and design-oriented model.

### 4.2 The Physical Body Is a Dynamic Instantiation

The physical body defines:

* rigid bodies
* collision shapes
* joints
* mass distribution
* contact behavior

It is a simulation-oriented model.

### 4.3 Why They Must Be Distinguished

The abstract node graph should not be mistaken for a direct list of Box2D bodies.

For example:

* some abstract nodes are only anchors or references
* some physical joints may exist even if there is no visible node with the same name
* the physical body may need support geometry richer than the visible stickman drawing

This distinction is necessary for both correctness and clarity.

---

## 5. Box2D as the Physical Simulation Layer

The physical body will be simulated in **Box2D**.

### 5.1 Role of Box2D

Box2D is responsible for:

* rigid body integration
* contacts
* collisions
* joints
* response to external forces and impulses

### 5.2 What Box2D Does Not Replace

Box2D does **not** replace:

* locomotion intent
* CM-level locomotion logic
* procedural nominal animation
* support reasoning at the project level
* rendering

So Box2D is the physical substrate of the project, not the full architecture.

### 5.3 Box2D Bodies During the Nominal Procedural Regime

For V1, the Box2D articulated body should still exist during the nominal regime.

However, during nominal procedural motion it should not be the authoritative locomotion driver.

The preferred V1 policy is:

* the articulated Box2D body exists from the beginning
* during nominal procedural motion it is kept in **kinematic sync** with the current procedural pose
* when the system enters the physical regime, this synchronized body becomes the starting point for dynamic evolution

This is preferred over disabling or creating the body only on demand because it preserves:

* spatial continuity
* articulated configuration continuity
* a cleaner procedural-to-physical transition

In other words, V1 uses Box2D during nominal motion as a synchronized physical representation, not as the authoritative source of locomotion.

### 5.4 Previous Procedural State

Because the project may switch from procedural to physical authority at any simulation step, the Model should retain a **previous procedural state**.

This stored state should conceptually include:

* previous procedural joint angles
* previous node positions
* previous global placement
* previous procedural CM state
* the corresponding simulation timestamp

This information is needed to estimate:

* segment angular velocities
* body linear velocities
* initial transition velocities when entering physical mode

without forcing the project to guess them from only one pose snapshot.

---

## 6. Physical Segment Set

The physical character should be built from a small set of rigid segments.

For V1, the most reasonable physical decomposition is:

* one **head body**
* one **upper torso body**
* one **lower torso / pelvis body**
* one **upper arm left**
* one **lower arm left**
* one **upper arm right**
* one **lower arm right**
* one **upper leg left**
* one **lower leg left**
* one **foot left**
* one **upper leg right**
* one **lower leg right**
* one **foot right**

### 6.1 Why the Torso Is Not a Three-Body Chain in V1

The abstract stickman uses:

* `torso_bottom`
* `torso_center`
* `torso_top`

For V1 physics, however, a simplified torso with:

* one lower torso body
* one upper torso body

is a better compromise between:

* controllability
* stability
* compatibility with arm and leg attachment

This does not contradict the abstract stickman.
It is a simplification of the physical representation.

### 6.2 Why Feet Are Explicit Physical Bodies

Unlike the abstract stickman, the physical model must include explicit feet.

This is necessary because:

* support cannot remain purely punctual if the system is expected to handle balance and recovery credibly
* contact geometry strongly affects stability
* capture and recovery logic depend on available support

This is consistent with [Capturability-based Analysis and Control of Legged Locomotion, Part 1: Theory and Application to Three Simple Gait Models.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Capturability-based%20Analysis%20and%20Control%20of%20Legged%20Locomotion,%20Part%201:%20Theory%20and%20Application%20to%20Three%20Simple%20Gait%20Models.pdf), which explicitly shows that capturability changes with finite foot support.

### 6.3 Abstract-to-Physical Mapping

The abstract and physical models are related, but not identical.

A useful V1 mapping summary is:

* `head_top` -> top reference of the **head body**
* `torso_top` -> upper reference region of the **upper torso body**
* `torso_center` -> reference point inside the **upper torso body**, not a separate body
* `torso_bottom` -> lower reference region of the **lower torso / pelvis body**
* `elbow_left` / `elbow_right` -> elbow joints between upper and lower arm bodies
* `wrist_left` / `wrist_right` -> distal endpoints of the lower arm bodies
* `knee_left` / `knee_right` -> knee joints between upper and lower leg bodies
* `ankle_left` / `ankle_right` -> ankle joints between lower leg and foot bodies

This mapping is important because the abstract node graph is used for geometry and reconstruction, while the physical body model is used for Box2D simulation.

---

## 7. Joint Topology

The physical bodies should be connected with articulated joints.

### 7.1 Main Joint Set

The required physical joints are:

* neck
* spine / torso joint
* shoulder left
* elbow left
* shoulder right
* elbow right
* hip left
* knee left
* ankle left
* hip right
* knee right
* ankle right

### 7.2 Joint Type

For V1, the natural default is:

* **revolute joints** for all articulated body connections

This matches the intended planar nature of the character.

### 7.3 Why Shoulders and Hips Exist Physically

The abstract stickman does not expose explicit shoulder or hip nodes.
The physical model should still use shoulder and hip joints because:

* arms need an actual attachment point to the torso
* legs need an actual attachment point to the pelvis/lower torso
* physics requires functional articulation, not only visible node labels

So the physical model is slightly richer than the visible graph where needed.

---

## 8. Joint Limits

All physical joints that represent articulated body motion must have **joint limits**.

### 8.1 Why Joint Limits Are Mandatory

Without limits, the physical body may produce:

* backward-bending knees
* impossible elbow postures
* implausible ankle rotations
* extreme torso folding

These would destroy both physical credibility and visual readability.

### 8.2 V1 Policy

For V1, all main joints should have:

* finite angular ranges
* anatomically plausible but simplified limits

Exact numeric values may be tuned later, but the existence of limits is not optional.

---

## 9. Collision Shapes

Each physical segment needs a collision shape that is simple enough for Box2D and meaningful enough for contact behavior.

### 9.1 Recommended Shape Strategy for V1

The recommended V1 strategy is:

* head as **circle**
* torso segments as **boxes or capsule-like boxes**
* limbs as **thin boxes**
* feet as **short horizontal support boxes**

### 9.2 Why Simplicity Is Preferred

The purpose of V1 is not detailed anatomical collision.
The purpose is:

* stable simulation
* meaningful support
* manageable tuning

So the shape set should remain simple.

---

## 10. Foot and Support Geometry

This is one of the most important design decisions of the physical body model.

### 10.1 V1 Foot Choice

For V1, the project should use a **finite foot of simple geometry**.

This means:

* each foot has non-zero length
* each foot can provide a small but real support region
* support is richer than a point foot

### 10.2 Why Not Point Feet

A point foot is simpler, but too restrictive for the intended project because:

* it provides a poor support description
* it makes recovery logic more fragile
* it makes the support base too impoverished
* it limits future stepping and capturability reasoning

This is consistent with [Capture Point: A Step toward Humanoid Push Recovery.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Capture%20Point:%20A%20Step%20toward%20Humanoid%20Push%20Recovery.pdf) and [Capturability-based Analysis and Control of Legged Locomotion, Part 1: Theory and Application to Three Simple Gait Models.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Capturability-based%20Analysis%20and%20Control%20of%20Legged%20Locomotion,%20Part%201:%20Theory%20and%20Application%20to%20Three%20Simple%20Gait%20Models.pdf), both of which distinguish the behavior of point-foot and finite-foot support models.

### 10.3 Why Not Full Heel/Toe Yet

A heel/toe foot model would be more realistic, but it would also require:

* richer contact-state logic
* more complex support transitions
* more detailed stepping state logic
* more tuning effort

For V1, this is too expensive.

### 10.4 Future Foot Evolution

The finite foot model should be chosen so that it can later evolve toward:

* heel-strike / toe-off logic
* heel/toe segmented support
* richer center-of-pressure reasoning

without replacing the whole architecture.

### 10.5 V1 Foot Size Convention

For V1, the exact numeric foot dimensions may still be tuned, but the foot should be understood as a short support segment whose horizontal extent is explicitly non-zero.

Conceptually, its length should be expressed relative to `L_rest`, not in unrelated ad hoc units.

---

## 11. Contact Policy

The physical model must distinguish between different kinds of contact.

### 11.1 Support-Relevant Contact

The following contacts are especially important:

* foot-ground contact during stance
* foot-terrain contact during stepping
* body-ground contact after falling

These contacts influence:

* support state
* falling state
* grounded stabilization
* get-up preconditions

### 11.2 Non-Support Contact

Other contacts may occur without being interpreted as proper support, for example:

* torso hitting the ground during a fall
* arm contact during collapse

These still matter physically, but they do not always define locomotor support in the same sense as a stance foot.

---

## 12. Collision Filtering

Collision filtering is required from the start.

### 12.1 Environment Contacts

The character should collide with:

* ground
* terrain
* obstacles

### 12.2 Self-Collision Policy for V1

For V1, the recommended default is to **ignore most self-collision** inside the character.

This is justified because:

* full self-collision adds complexity
* it is not necessary for the main goals of V1
* it can destabilize a first implementation unnecessarily

### 12.3 Why Filtering Must Be Explicit

Without clear collision filtering, the physical model may suffer from:

* irrelevant internal contacts
* solver instability
* noisy interpretation of support

So filtering is part of the physical model, not an implementation afterthought.

---

## 13. Mass and Inertia Modeling

The physical body must have masses and inertias.

### 13.1 V1 Philosophy

For V1, mass distribution should be:

* simplified
* physically plausible
* not necessarily anthropometrically exact

This is acceptable because the project aims at a credible hybrid character, not a biomedical human replica.

### 13.2 Why Mass Modeling Matters

Mass distribution directly affects:

* reaction to perturbation
* falling behavior
* balance response
* emergent physical CM

So it cannot be ignored indefinitely.

### 13.3 Relation to the Emergent CM

Once the body is in physical mode, the project must be able to interpret the physical motion through the **emergent CM** of the articulated body.

This means the physical body model should support later computation of:

* overall physical center of mass
* comparison between procedural CM target and physical CM result

The exact recovery metric will be defined later, but the physical body must already make it possible.

---

## 14. Procedural-to-Physical Transition Principle

Since V1 uses a switch-based hybrid architecture, the transition into physical mode is one of the most important responsibilities of the physical body model.

### 14.1 General Principle

The physical body must begin from the **current actual character state**, not from a reset pose.

This means the transition should preserve as much as possible:

* current body pose
* current body orientation
* current contact/support context
* current motion direction

### 14.2 Why This Matters

If physical mode starts from an arbitrary predefined pose, then:

* perturbation stops looking continuous
* recovery credibility is weakened
* falling becomes visually artificial

This would contradict the principles of [06_Transitions_And_Continuity.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/06_Transitions_And_Continuity.md).

### 14.3 V1 Practical Interpretation

For V1, the physical body should be initialized or synchronized from the current procedural state at the transition moment.

That synchronization should aim to preserve:

* articulated configuration
* global placement
* as much velocity information as is reasonably available

The preferred V1 mechanism is:

* use the current procedural state for pose and placement
* use the stored previous procedural state to estimate linear and angular velocities by finite difference
* switch the synchronized Box2D body from nominal kinematic sync into dynamic physical authority

The exact step ordering of this transition is detailed in [12_Update_Loop_And_Timing.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/12_Update_Loop_And_Timing.md).

---

## 15. Physical-to-Procedural Return Principle

The physical body is not the permanent owner of the character in V1.

After perturbation, fall, or grounded stabilization, the system may later return to procedural control.

### 15.1 Return Is Not an Immediate Snap

Return to procedural motion should not happen as:

* direct teleport to a canonical pose
* abrupt removal of physical history

Instead, the project should later use:

* state capture
* grounded stabilization
* controlled procedural re-entry

### 15.2 Why This Matters Here

Even if the exact re-entry logic belongs to later documents, the physical body model must already be designed so that:

* physical state can be inspected
* grounded configurations can be recognized
* a later procedural phase can be reattached to the current body reality

---

## 16. Relation to Future Reconstruction and IK

The physical body model sets hard constraints on the future reconstruction and IK process.

In particular, later reconstruction must respect:

* the physical joint topology
* the physical joint limits
* the support geometry
* the segment lengths implied by the physical bodies

This means reconstruction cannot be designed independently of the physical model.

At the same time, the IK and reconstruction document will remain responsible for:

* producing nominal procedural body targets
* deciding posture priorities
* converting CM and support objectives into articulated targets

So the relation is:

* this document defines the **physical body that can exist**
* the reconstruction document will define the **target body organization that should be requested**

---

## 17. Main Failure Modes to Avoid

The following design failures should be explicitly avoided:

* using point feet while still expecting rich support behavior
* omitting joint limits
* confusing abstract visible nodes with physical bodies one-to-one
* overcomplicating the physical body in V1
* letting self-collision create unnecessary instability
* resetting the body artificially when entering physical mode
* hard-coding the model to flat-ground assumptions in a way that would block later terrain extension

These are structural mistakes, not just implementation details.

---

## 18. What Is Deferred to Later Documents

This document does not yet define:

* the full IK solver
* the full reconstruction logic
* the exact recoverability criterion
* the exact timing of transitions
* the exact update-loop order
* the final validation metrics

These belong to the next design documents.

---

## 19. Summary of Main Takeaways

1. V1 uses a switch-based hybrid architecture: procedural/kinematic in nominal motion, physical Box2D behavior under strong perturbation and failure-related regimes.
2. The physical body is derived from, but not identical to, the abstract stickman model.
3. The physical model should use a small articulated body set with explicit feet, revolute joints, and mandatory joint limits.
4. A finite foot of simple geometry is preferred over a point foot for V1.
5. Collision filtering and a clear contact policy are mandatory parts of the physical model.
6. The physical model must support credible procedural-to-physical transition without arbitrary reset.
7. The architecture should remain open to future evolution toward richer feet and an always-physical character architecture.
