# CM and Locomotion Models

## 1. Role of This Document

This document defines the **global locomotion logic** of the project through the behavior of the **center of mass (CM)**.

Its purpose is to answer the following questions:

* What is the CM in this project?
* Which variables define its state?
* How should the CM behave in different locomotion regimes?
* Which parts of that behavior come from direct design choice, and which are inspired by the literature?
* What continuity and support constraints must always be respected?
* How should the model remain extensible to future terrain complexity?

This document does **not** yet define:

* the full hybrid switching logic between procedural and physical modes
* joint-level torque control
* software architecture
* rendering logic

Those topics belong to later documents.

---

## 2. Why the CM Is Central

The project adopts a system-level view of locomotion.
This means that locomotion is not treated primarily as a sequence of local joint rotations, but as the controlled transport of the body as a whole.

This perspective is strongly supported by [The Motion of Body Center of Mass During Walking](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf), which emphasizes that the body as a whole can be mechanically represented by its center of mass and that this perspective helps organize locomotion at a meaningful global level.

In the present project, the CM is therefore used as a **global locomotion variable**.
It is not introduced only as a measurement.
It is used to organize body transport, support transitions, and the plausibility of the overall motion.

This decision is not a claim that the real body can be reduced entirely to one point.
It is a modeling choice: the CM is used as the main global abstraction around which the rest of the stickman will later be organized.

---

## 3. Two Interpretations of the CM

One of the most important ideas of the project is that the CM does not play exactly the same role in every regime.

### 3.1 Controlled CM

During nominal procedural locomotion, the CM is treated primarily as a **controlled state**.

This means:

* a locomotion mode defines how the CM is supposed to evolve
* support logic and foot placement react to that evolution
* the articulated body is reconstructed around that target behavior

This is the main sense in which the project is **CM-first**.

### 3.2 Emergent CM

During strongly perturbed or physically reactive phases, the body may no longer follow only the nominal procedural CM target.
At that point, the physical state of the body evolves under:

* gravity
* contacts
* impacts
* control torques

and the resulting CM becomes more **emergent** than directly prescribed.

This distinction is essential:

* in nominal locomotion, the CM is mostly a **driver**
* in strongly physical motion, the CM is mostly a **result**

Later documents will define exactly how the system switches between these interpretations.

---

## 4. CM State Definition

The CM should not be understood only as a point in space.
For locomotion purposes, it must be understood as a **state**.

At the conceptual level, the CM state includes:

* **position**
* **velocity**
* **acceleration**
* **target vertical operating level**
* **current locomotion regime**
* **current support relation**

This is the minimum information needed to reason about:

* where the body is
* where it is going
* how fast it is changing
* whether the current support is still compatible with the motion

### 4.1 Why Velocity Matters

The importance of CoM position and velocity together is particularly clear in [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), where balance feedback uses both CoM position and CoM velocity to adapt future support.

This means that a static geometric CM target is not enough.
The temporal state of the CM matters.

### 4.2 Why Acceleration Matters

Acceleration matters because:

* it reflects how aggressively the body is being redirected
* it influences support demands
* it affects the smoothness of transitions
* it helps distinguish calm locomotion from impact-like events

The project may not always prescribe acceleration explicitly, but it must remain conceptually aware of it.

---

## 5. Continuity of the CM

The CM cannot jump arbitrarily from one state to another if the motion is expected to look believable.

### 5.1 What Continuity Means Here

In nominal locomotion, the CM should evolve with sufficient continuity in:

* position
* velocity
* and, when possible, acceleration

This is necessary because abrupt CM discontinuities would make the following layers unstable or implausible:

* foot placement
* support transitions
* articulated reconstruction
* visual continuity of the body

### 5.2 Not All Motion Must Be Perfectly Smooth

Continuity does **not** mean that every event must be perfectly smooth.
Impacts and hard landings may legitimately create sharp changes in velocity or acceleration.

The correct interpretation is:

* smoothness is required in controlled locomotion
* non-smoothness is acceptable when demanded by real contact events, perturbations, or collisions

### 5.3 Literature Context

[Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf) is relevant here because it emphasizes springs, interpolation, synchronized blends, and physically meaningful transitions instead of abrupt pose replacement.

The project should follow the same lesson:
CM continuity should come mainly from model structure and controlled transitions, not from cosmetic smoothing alone.

---

## 6. Support and Capturability

The CM cannot be discussed independently of support.

This is one of the major lessons that appears across the literature:

* [The Motion of Body Center of Mass During Walking](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf) links CoM motion to balance and support
* [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf) links CoM state to future point of support
* [Kuo 2007](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Kuo2007-Six-Determinants-Gait-Inverted-Pendulum.pdf) highlights the importance of the transition from one support leg to the next

For this reason, every locomotion regime in this project should be interpreted together with a support relation.

### 6.1 Basic Principle

A CM state is only meaningful if the body has a plausible way to support or recapture it.

This means that the system must later be able to answer questions such as:

* Is the current support enough?
* Is a support transition needed?
* Is the next step reachable?
* Is the perturbation still recoverable?

These questions belong to later control documents, but the CM model must already be compatible with them.

### 6.2 Future Terrain Extension

This is also where future terrain complexity becomes important.
The CM model should not be written as if support always meant a single flat horizontal ground at fixed height.

Even if the first implementation will likely use simple flat terrain, the conceptual model must already remain valid for future cases such as:

* slopes
* uneven surfaces
* discontinuous ground
* locally rough terrain

This requirement is consistent with [The Spring-Mass Model and Other Reductionist Models](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Spring-Mass%20Model%20and%20Other%20Reductionist%20Models.pdf), which explicitly studies reduced locomotion models on inclines and shows that locomotor dynamics change with surface inclination.

The present document therefore speaks in terms of:

* support conditions
* support surfaces
* support transitions

and not only in terms of flat-ground contact.

---

## 7. Locomotion Regimes

The CM does not follow one universal law for all forms of motion.
Different behaviors require different conceptual models.

This is one of the most important points of the entire document.

### 7.1 Standing

Standing should not be modeled as a degenerate version of walking.
It is a distinct regime of **postural regulation**.

In standing:

* there is no repeated step-to-step transport
* the main objective is to keep the body controllable above its support
* small oscillations are acceptable and expected
* perturbations may trigger corrective behavior

So the CM in standing is primarily regulated relative to the current support base.

#### 7.1.1 Why This Matters

If standing were treated as “walking with zero speed,” the model would miss an important distinction:
the logic of balance regulation is not the same as the logic of pendular transport.

### 7.2 Walking

Walking should be modeled primarily through an **inverted pendulum interpretation** of the CM.

This is strongly supported by [Kuo 2007](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Kuo2007-Six-Determinants-Gait-Inverted-Pendulum.pdf), which argues that the stance leg behaves like an inverted pendulum during single support and that the body is transported economically in this way.

In this project, the walking CM model should therefore include the following ideas:

* during single support, the body passes over the stance leg
* the CM follows an arc-like trajectory rather than an arbitrary flat path
* walking consists not only of single support, but also of transition from one support to the next

#### 7.2.1 Step-to-Step Transition

[Kuo 2007](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Kuo2007-Six-Determinants-Gait-Inverted-Pendulum.pdf) is especially important because it insists that walking is not explained fully by the pendular arc alone.
The **step-to-step transition** is also essential.

That means that, in this project, a walking model cannot stop at:

* “the CM follows an arc”

It must also include:

* “the CM must be redirected from one support phase to the next”

This is one of the main reasons why support planning and CM planning must remain tightly linked.

### 7.3 Running

Running should be modeled primarily through a **spring-mass interpretation** of the CM.

This is consistent with [The Spring-Mass Model and Other Reductionist Models](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Spring-Mass%20Model%20and%20Other%20Reductionist%20Models.pdf), as well as with classical running biomechanics literature such as [Biomechanics of Running Gait.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Biomechanics%20of%20Running%20Gait.pdf).

In running:

* there is an aerial phase
* the support phase is compliant rather than purely rigid
* the CM lowers toward mid-stance and then rises again
* elastic-like compression and release shape the motion

So unlike walking, running is not primarily pendular.

#### 7.3.1 Important Consequence

Vertical motion in running is not merely an error to minimize.
It is part of the gait structure itself.

This point is important, because otherwise one might incorrectly try to keep the CM too flat in all locomotion modes.

### 7.4 Jumping

Jumping should be treated as a phase-based regime rather than as a steady periodic gait.

At minimum, the conceptual jump sequence includes:

* loading
* push-off
* flight
* landing

The CM model should reflect this sequence explicitly.

During flight, the CM is governed mainly by ballistic motion.
During loading and landing, the CM is governed by support, force production, and energy absorption.

### 7.5 Landing

Landing should not be treated as a trivial end of jumping.
It is a transition regime in its own right.

In landing:

* the CM must be decelerated
* support must be re-established
* impact energy must be absorbed
* the system must decide whether the body remains recoverable

Landing therefore forms an important bridge between locomotion and perturbation logic.

### 7.6 Crouching

Crouching is not a new gait in the same sense as walking or running.
It is a **change of operating posture**.

In crouching:

* the nominal CM height is lower
* the body geometry is more compact
* stability and support margins may change
* later jump and recovery behavior may also change

So crouching should be treated as a posture regime that modifies the nominal operating point of the CM.

### 7.7 Recovery

Recovery should be treated as a dedicated locomotion-related regime rather than as a simple blend back to the nearest stable pose.

In recovery:

* the body attempts to regain a controllable support condition
* the CM must be brought back into a capturable region
* the system may need postural correction, support adaptation, or both

This interpretation is consistent with the support-feedback spirit of [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf), where support adjustment is part of balance restoration.

### 7.8 Falling

Falling is a legitimate state of the system, not an animation failure.

From the CM perspective, falling begins when:

* the current support is no longer sufficient
* the perturbation is no longer capturable
* the body cannot be redirected back into a controllable regime in time

This does not mean the project stops controlling the character completely.
It means the model must accept that locomotion has transitioned from balance-preserving behavior to loss-of-balance behavior.

---

## 8. CM Operating Height by Regime

The project should not assume that the CM has one single nominal height valid for every locomotion regime.

This point matters because the global body posture and the support demands are not the same in:

* standing
* walking
* running
* crouching
* jumping
* landing
* falling

So, in addition to position and velocity, each regime should also be understood through a characteristic **CM operating height**.

### 8.1 Standing

In standing, the CM operates around its nominal upright reference height.
This is the calmest and most neutral operating level of the system.

### 8.2 Walking

In walking, the CM operating height remains relatively close to the standing reference, but with moderate vertical oscillation imposed by pendular transport.

The important point is that walking should not flatten the CM artificially.
The body should remain relatively high over the stance leg while still undergoing step-to-step redirection.

### 8.3 Running

In running, the effective CM operating behavior is more compliant.
Compared to walking, the system should allow:

* deeper compression during stance
* a lower mid-stance position
* larger vertical oscillation

This is one of the reasons why running should not be modeled with the same vertical operating assumptions as walking.

### 8.4 Crouching

Crouching explicitly lowers the CM operating height.

This is not just a visual posture change.
It changes:

* body geometry
* support margins
* future jump preparation
* recovery possibilities

### 8.5 Jump Preparation and Push-Off

During jump preparation, the CM operating height is typically lowered before push-off.
This loading phase is part of the preparation for force production.

### 8.6 Flight

During flight, the CM is no longer constrained by support.
Its height is therefore no longer an operating setpoint in the same sense as standing or crouching.

Instead, it is governed mainly by ballistic evolution.

### 8.7 Landing

During landing, the CM typically drops into a lower temporary operating level as the body absorbs impact and attempts to recover control.
This lowered phase should not be confused with failure.
It is part of the landing and stabilization process.

### 8.8 Falling

In falling, there is no stable controlled operating height.
The CM is no longer maintaining a locomotion-support relation in the nominal sense.

### 8.9 Why This Section Matters

This section exists to make explicit that each locomotion regime is characterized not only by:

* a motion type
* a support pattern
* a dynamic interpretation

but also by a different **vertical operating logic** of the CM.

This distinction will become important later when defining:

* regime transitions
* articulated reconstruction targets
* physical tracking priorities
* recovery behavior

---

## 9. Regime Summary Table

The following table summarizes the intended interpretation of the CM in each major regime.

| Regime | Main CM interpretation | Main support interpretation |
|---|---|---|
| Standing | postural regulation | maintain controllable support |
| Walking | inverted pendulum + step-to-step redirection | alternate support and redirect CM |
| Running | spring-mass + aerial phase | compliant support and take-off / landing |
| Jumping | loading, push-off, flight, landing | temporary support loss and regain |
| Crouching | lowered operating posture | modified support geometry and margins |
| Recovery | recapture of controllable CM state | corrective support or posture adaptation |
| Falling | loss of capturable locomotion state | support no longer sufficient |

This table does not replace the detailed text above, but it helps summarize the logic of the document.

---

## 10. Ground Assumptions and Future Terrain Extension

### 9.1 First Implementation Simplicity

The first implementation of the project will likely begin on a simple ground model.
This is acceptable and practical.

### 9.2 Conceptual Model Must Remain Terrain-General

However, the locomotion model should already be written in a way that can later extend toward:

* rough terrain
* sloped terrain
* uneven terrain
* discontinuous terrain

This means that even in the present document:

* support should not be treated as “a constant horizontal line forever”
* locomotion regimes should not be defined only for flat-ground kinematics
* support transitions should remain conceptually valid on nontrivial surfaces

### 9.3 Why This Is Justified

[The Spring-Mass Model and Other Reductionist Models](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Spring-Mass%20Model%20and%20Other%20Reductionist%20Models.pdf) explicitly shows that reduced CoM models can be extended to **inclines** and that surface slope changes:

* stride-related kinematics
* vertical oscillation
* stance behavior

Similarly, [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf) explicitly demonstrates robustness to:

* unexpected steps
* slopes
* terrain variation

This means that terrain complexity is not an external afterthought.
It is a natural future extension of the same family of locomotion models already used here.

### 9.4 Practical Consequence

The correct strategy for the project is:

* **simple ground in the first prototype**
* **terrain-extensible locomotion concepts from the beginning**

That is why this document speaks in terms of support surfaces and support conditions rather than tying every regime to a single horizontal reference line.

---

## 11. What Is Defined Here vs Later

### 10.1 Defined Here

This document defines:

* the central role of the CM
* the distinction between controlled and emergent CM
* the CM state variables at conceptual level
* the required continuity of the CM
* the major locomotion regimes
* the relation between locomotion and support
* the requirement of future terrain extensibility

### 10.2 Deferred to Later Documents

This document does **not** yet define:

* exact switching rules between regimes
* exact recovery criteria
* exact falling thresholds
* foot placement algorithms
* joint reconstruction methods
* PD tracking torques
* software architecture

These topics will be introduced later, once the conceptual motion regimes are fixed.

---

## 12. Summary of Main Takeaways

The main points of this document are the following:

1. The project treats locomotion primarily through the motion of the body as a whole, represented by the CM.
2. The CM plays two different roles depending on regime: controlled in nominal locomotion, more emergent in strongly physical behavior.
3. CM motion must remain constrained by support, capturability, and body plausibility.
4. Different locomotion regimes require different conceptual models:
   - standing as postural regulation
   - walking as inverted-pendulum-like transport with step-to-step transition
   - running as spring-mass-like motion
   - jumping and landing as phase-based motion
   - crouching as a lowered operating posture
   - recovery as recapture of control
   - falling as legitimate loss of control
5. The conceptual model should remain extensible to future rough, sloped, or discontinuous terrain even if the first implementation begins on simple flat ground.

This document therefore establishes the **conceptual dynamics layer** of the project.
The next document should define how these regimes interact inside the hybrid system.
