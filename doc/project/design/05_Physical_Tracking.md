# Physical Tracking

## 1. Role of This Document

This document defines the conceptual role of the **Physical Tracking Layer** of the project.

Its purpose is to explain how:

* a procedural target pose
* a physically simulated body
* and a set of control torques

can be combined into a coherent hybrid behavior.

This document does **not** yet define:

* exact implementation classes
* exact numerical solver details
* exact gain values for every joint
* exact rigid-body equations

Instead, it explains the conceptual mechanism that allows the physical body to remain linked to procedural intention.

---

## 2. Why Physical Tracking Is Needed

The project is neither:

* a purely kinematic animation player

nor:

* a purely passive ragdoll

The whole point of the hybrid system is to preserve intention while still allowing physical response.

This creates a problem:

* if the body follows procedural motion too rigidly, perturbations stop looking physical
* if the body follows physics alone, the character quickly loses intentional structure

The Physical Tracking Layer exists to solve exactly this problem.

It acts as a bridge between:

* what the character is trying to do
* what the body is physically doing

This hybrid idea is strongly compatible with the spirit of:

* [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), where body motion is driven through target angles and control torques
* [Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf), where procedural animation and ragdoll-like physics are treated as cooperating tools
* [Advanced Character Physics](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Advanced%20Character%20Physics.pdf), where articulated physical behavior remains controllable through constraints

---

## 3. Procedural Pose as Reference

The Physical Tracking Layer assumes that another part of the system has already produced a **procedural target**.

This target may include:

* target joint angles
* target angular velocities
* target trunk orientation
* target body organization associated with the current locomotion regime

### 3.1 A Reference, Not an Absolute Command

This is an important distinction:

the procedural pose is a **reference**, not an absolute command that the physical body must satisfy perfectly at all costs.

That means:

* the target says where the body would like to be
* the physical body may deviate from that target
* the tracking layer tries to reduce the deviation without denying physics

This idea is central to the whole hybrid design.

---

## 4. Physical Body as Dynamic System

The body that receives the target pose is a dynamic system.

That means it evolves under:

* mass
* inertia
* gravity
* contact
* impact
* external perturbation
* internal actuation

So the physical body cannot always exactly follow the procedural target.

### 4.1 Consequence

The body is not simply “set” into the target pose.
Instead, it is **driven toward** that pose through physical influence.

This difference is essential.

If the body were directly teleported to the target:

* perturbations would lose their meaning
* contact realism would break
* support logic would become artificial

The role of physical tracking is therefore to preserve dynamic behavior while still preserving intention.

---

## 5. Joint PD Tracking Principle

The most direct conceptual method proposed for this layer is **joint PD tracking**.

### 5.1 Core Formula

The tracking torque can be written conceptually as:

```text
tau = K_p (theta_target - theta_current) + K_v (theta_dot_target - theta_dot_current)
```

where:

* `tau` is the control torque applied at the joint
* `theta_target` is the procedural target angle
* `theta_current` is the current physical joint angle
* `theta_dot_target` is the target angular velocity
* `theta_dot_current` is the current angular velocity
* `K_p` is the proportional gain
* `K_v` is the damping / velocity gain

### 5.2 Important Terminology Clarification

This expression defines a **joint torque** or **control moment**.
It should **not** be called `momentum`.

This distinction matters:

* **momentum** is a physical quantity such as linear or angular momentum
* **torque** is the actuation used here to influence the body

So in this project, the correct terminology is:

* `joint torque`
* `control torque`
* or `joint moment`

not `momentum`

### 5.3 Intuition of the Formula

The formula combines two types of correction:

* the first term tries to reduce the angular error
* the second term tries to reduce the angular velocity error

In simple language:

* if the joint is too far from the target, the controller pushes it back
* if it is moving too differently from the target, the controller damps or redirects it

This is what makes PD tracking such a natural bridge between:

* target motion
* dynamic body response

---

## 6. Why PD Tracking Makes Sense Here

PD tracking is especially appropriate for this project because it is:

* simple
* interpretable
* physically meaningful
* compatible with articulated simulation
* easy to modulate depending on the regime

This kind of control is also conceptually close to the joint-angle tracking strategy used in [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), where target joint angles are pursued through proportional-derivative control.

The value of PD tracking in this project is not that it solves everything by itself.
Its value is that it provides a robust and understandable link between:

* desired articulated organization
* actual physical articulated motion

---

## 7. Active Ragdoll Interpretation

The Physical Tracking Layer can be understood as the foundation of an **active-ragdoll-like behavior**.

### 7.1 Passive vs Active Ragdoll

A passive ragdoll:

* receives forces
* collapses under physics
* has no strong internal intention

An active ragdoll:

* remains physically simulated
* but still tries to follow meaningful body targets

The project is much closer to the second case.

### 7.2 Why This Matters

This is exactly what the hybrid project needs:

* not a frozen procedural puppet
* not a dead passive ragdoll
* but a physically reactive body that still tries to behave like the same character

This interpretation is strongly aligned with the spirit of [Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf), where active ragdoll ideas and animation-following physical bodies are treated as useful practical tools.

---

## 8. Regime-Dependent Strength of Tracking

The Physical Tracking Layer should not behave identically in every regime.

This is one of the most important design principles of the whole document.

### 8.1 Nominal Procedural Regime

In nominal procedural locomotion:

* tracking should be relatively strong
* the body should stay close to the intended organization
* readability and intentional motion are priorities

### 8.2 Physical Reactive Regime

Under strong perturbation:

* tracking should be weaker or more permissive
* the perturbation must remain visible
* the body should not fake perfect immunity to external forces

### 8.3 Recovery Regime

In recovery:

* tracking should help the body return toward control
* but should not erase the dynamics of the disturbance too early

This regime is particularly important because it must reconnect the physical body with procedural intent.

### 8.4 Falling Regime

In falling:

* tracking should generally become weak, selective, or protective
* the goal is no longer to preserve a perfect locomotion pose

The system may still preserve some body organization, but it should not force implausible resistance against an unrecoverable fall.

### 8.5 Procedural Get-Up Regime

In get-up:

* tracking becomes stronger again
* the system reintroduces a more intentional target body organization

This is one of the main mechanisms through which the character transitions back toward procedural motion.

---

## 9. Tracking Priorities Across the Body

Not all body parts need the same tracking priority.

### 9.1 Why Uniform Tracking Is Not Ideal

If every joint is tracked with the same conceptual importance:

* the body may become too rigid
* the motion may feel artificial
* important joints may not receive the priority they deserve

### 9.2 Expected Priority Differences

At a conceptual level, the system may later give greater importance to:

* torso organization
* support-side leg organization
* joints directly involved in balance preservation

while allowing more freedom in:

* swing-side articulation
* secondary expressive motion
* upper-limb reaction

This document does not assign final priorities yet.
It only establishes that the architecture should support them.

---

## 10. Tracking Is Not the Same as Balance

This distinction is essential.

The fact that the body is tracking a target pose does **not** mean the body is balanced.

### 10.1 Why This Matters

A pose can be:

* visually reasonable
* locally well tracked

and still be:

* globally unstable
* incompatible with support
* dynamically unrecoverable

This is why the project must keep physical tracking distinct from:

* CM control
* support reasoning
* foot placement planning

### 10.2 Practical Consequence

If support and balance logic disagree with the procedural pose:

* support logic may need to dominate
* the target pose may need to be relaxed or modified

So physical tracking must remain subordinate to whole-body plausibility.

---

## 11. Tracking and Contact Must Remain Compatible

The physical body interacts with the environment through contact.

For that reason, the tracking layer must remain compatible with:

* ground support
* non-penetration
* stance consistency
* later terrain-dependent contact

### 11.1 Why This Matters

If the tracking layer tries to enforce a pose that contradicts contact:

* feet may appear to slide
* support may be visually or physically broken
* recovery may become implausible

This concern is conceptually related to the issues highlighted by [Footskate Cleanup for Motion Capture Editing.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Footskate%20cleanup%20for%20motion%20capture%20editing.pdf), even though the present project is not a motion-capture cleanup system.

### 11.2 Practical Consequence

Tracking should always be interpreted under contact constraints, not independently of them.

---

## 12. Failure Modes of Physical Tracking

The Physical Tracking Layer can fail in several recognizable ways.

### 12.1 Tracking Too Strong

If gains are too high:

* the body may become unnaturally rigid
* perturbations may stop looking physical
* oscillations or instability may appear

### 12.2 Tracking Too Weak

If gains are too low:

* the character may lose intentional structure
* recovery may become too soft or ineffective
* the body may look passive rather than active

### 12.3 Impossible Target Pose

If the target pose is incompatible with:

* current support
* current momentum
* current contact geometry

then the tracking layer may end up fighting physics instead of supporting it.

### 12.4 Uniform Tracking Everywhere

If all joints are treated identically:

* the motion may fail to prioritize balance-critical structure
* the body may behave in an unnatural all-or-nothing way

These failure modes justify the need for:

* regime-dependent tracking
* support-aware tracking
* body-priority-aware tracking

---

## 13. What This Layer Should Achieve

The Physical Tracking Layer should make the following possible:

* a physical body that still expresses procedural intention
* visible reaction to perturbation
* support of recovery without teleportation
* permissive transition toward falling when recovery fails
* structured return toward get-up and nominal control afterward

This is the true success criterion of the layer.

---

## 14. What Is Deferred to Later Documents

This document does **not** yet define:

* exact gain values
* exact joint-by-joint tracking priorities
* exact torque saturation rules
* exact rigid-body contact solver behavior
* exact implementation interfaces

Those details belong to later design and implementation documents.

---

## 15. Summary of Main Takeaways

1. The physical body should not be teleported to a procedural pose; it should be driven toward it through control torques.
2. A PD tracking law is an appropriate conceptual mechanism for this purpose.
3. The result should be understood as active-ragdoll-like behavior rather than as passive ragdoll or pure kinematic animation.
4. Tracking strength must depend on the active regime.
5. Tracking alone does not guarantee balance; support and CM logic remain essential.
6. Tracking must remain compatible with contact.

This document therefore establishes the conceptual **physical tracking layer** of the project.
The next document should focus on transitions, continuity, and re-synchronization between regimes.
