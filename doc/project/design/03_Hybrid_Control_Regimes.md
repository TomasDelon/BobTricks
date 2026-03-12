# Hybrid Control Regimes

## 1. Role of This Document

This document defines the **hybrid regime structure** of the character system.

The previous documents established:

* the body model of the stickman
* the conceptual role of the CM
* the major locomotion regimes of the CM

What is still missing is the logic that explains:

* when the system remains procedural
* when it becomes physically reactive
* when it attempts recovery
* when it accepts falling
* how it returns from a grounded state back to nominal motion

This document exists to define that layer.

It does **not** yet define:

* the exact class architecture
* the exact implementation of torque controllers
* the exact mathematical threshold values

It defines the conceptual regime map of the project.

---

## 2. Why a Hybrid Regime Structure Is Necessary

The project does not aim to solve every situation with a single control method.

This is important for two reasons:

1. **pure procedural control** is strong for readable, intentional locomotion, but weak under large impacts or strong external disturbance
2. **pure physical simulation** is strong for reactive motion, but weak for maintaining intentional and expressive nominal movement

This tension is already visible in the literature:

* [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf) shows that robust locomotion needs feedback and support adaptation
* [Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf) clearly embraces mixed procedural, IK, spring, and ragdoll-like strategies
* [Advanced Character Physics](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Advanced%20Character%20Physics.pdf) supports the use of physically plausible articulated reactions rather than purely kinematic correction

The conclusion for this project is that the character should not stay in one universal mode at all times.
Instead, it should move between a small number of clearly defined control regimes.

---

## 3. The Main Regimes of the System

The hybrid system is built around six major regimes:

1. **Nominal Procedural Regime**
2. **Physical Reactive Regime**
3. **Recovery Regime**
4. **Falling Regime**
5. **Grounded Stabilization Regime**
6. **Procedural Get-Up Regime**

These regimes are not separate characters.
They are different control interpretations of the same character.

---

## 4. Nominal Procedural Regime

The nominal procedural regime is the default state of the character when it is behaving intentionally and remains in control.

### 4.1 Purpose

Its purpose is to generate:

* standing
* crouching
* walking
* running
* jumping under expected conditions
* landing under expected conditions

### 4.2 Main Characteristics

In this regime:

* the CM is treated primarily as a **controlled locomotion state**
* support logic follows the nominal locomotion model
* body posture is reconstructed around the intended motion
* continuity and readability are prioritized

### 4.3 What This Regime Is Good At

This regime is good at:

* intentional movement
* clean transitions between planned actions
* preserving the identity of the character
* biomechanically inspired but organized motion

### 4.4 What This Regime Is Not Good At

This regime should not be forced to resolve:

* large impacts
* large pushes
* major collisions
* strongly destabilizing contact events

When such events become dominant, another regime is needed.

---

## 5. Physical Reactive Regime

The physical reactive regime is entered when the environment takes over strongly enough that purely nominal procedural control is no longer sufficient.

### 5.1 Purpose

Its purpose is to let the body react credibly to:

* pushes
* impacts
* collisions
* destabilizing landings
* sudden support changes

### 5.2 Main Characteristics

In this regime:

* the articulated body evolves under physical effects and control torques
* the CM becomes more **emergent** than directly prescribed
* the character may still try to preserve intention, but physical plausibility has higher priority

### 5.3 Physical Does Not Mean Uncontrolled

This point is important.
The physical reactive regime is not the same thing as a passive ragdoll.

The system may still contain:

* stabilizing torques
* pose-tracking torques
* recovery-oriented objectives

The difference is that the body is no longer assumed to stay on its nominal procedural trajectory.

### 5.4 Relation to the Literature

This regime is conceptually consistent with:

* robust push-recovery ideas from [SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf)
* active-ragdoll and physically assisted animation logic suggested by [Animation Bootcamp](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf)
* articulated physical constraint handling in [Advanced Character Physics](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Advanced%20Character%20Physics.pdf)

---

## 6. Recovery Regime

The recovery regime is entered when the character has been destabilized, but the perturbation is still considered **recoverable**.

### 6.1 Purpose

Its purpose is to restore a controllable state without pretending that nothing happened.

The system should try to:

* reduce destabilizing momentum
* regain a viable support relation
* bring the CM back into a capturable region
* re-organize the body for return to nominal control

### 6.2 Why Recovery Is a Separate Regime

Recovery is not just:

* a visual blend
* a pose snap
* an immediate return to the nominal controller

It is a real transitional regime in which the body is still dealing with the consequences of disturbance.

### 6.3 Possible Recovery Means

Conceptually, recovery may involve:

* postural correction without stepping
* support adaptation through a recovery step
* support adaptation through multiple recovery steps
* controlled dissipation of excess motion
* reorientation toward a stable support state

This is important because not every perturbation should be handled in the same way.
For example:

* a small perturbation may be recoverable through posture only
* a medium perturbation may require one recovery step
* a larger but still recoverable perturbation may require several steps before the CM becomes controllable again

The exact implementation of these mechanisms belongs to later documents.
What matters here is that recovery is recognized as a distinct system phase.

---

## 7. Falling Regime

The falling regime is entered when the perturbation is no longer considered recoverable.

### 7.1 Purpose

Its purpose is to allow the character to fail credibly.

This includes:

* abandoning unrealistic attempts to remain upright
* transitioning into loss-of-balance dynamics
* contacting the ground in a believable way

### 7.2 Why Falling Must Be a Real Regime

Falling should not be treated as a bug or as a missing animation.

The whole project explicitly aims to support:

* perturbation
* failed recovery
* fall
* later return to intentional behavior

So falling must be part of the normal regime structure.

### 7.3 Controlled Failure

Even in falling, the system may still try to preserve some plausibility:

* reduce obviously unnatural collapse
* preserve believable whole-body reaction
* prepare for a grounded state that can later lead to a get-up sequence

This means that falling is not equivalent to “disable all control forever.”

---

## 8. Grounded Stabilization Regime

After a fall, the body should eventually reach a state where large instability has dissipated.

This is the grounded stabilization regime.

### 8.1 Purpose

Its purpose is to:

* settle the body after impact
* stop the large-scale collapse process
* identify a usable grounded configuration
* prepare the transition toward procedural get-up

### 8.2 Why This Regime Matters

Without a grounded stabilization phase, the system would have to jump directly from:

* falling

to

* getting up

That would be too abrupt conceptually and physically.

Grounded stabilization is therefore the bridge between failure and regained intention.

---

## 9. Procedural Get-Up Regime

Once the character reaches a suitable grounded state, it may re-enter a more intentional control mode through a get-up process.

### 9.1 Purpose

Its purpose is to:

* reorganize the body from a grounded pose
* return toward a valid upright state
* reconnect with nominal procedural locomotion

### 9.2 Why Get-Up Is Procedural Again

The project aims to give the character intentional, readable motion once control has been regained.
For that reason, getting up is conceptually better treated as a return toward procedural organization rather than as continued uncontrolled physics.

### 9.3 Get-Up Does Not Mean Reset

The get-up regime should not be understood as:

* instant teleportation to a standing pose
* hard reset of the character

Instead, it is the final part of the hybrid cycle:

* physical destabilization
* failure or stabilization
* controlled re-entry into intentional motion

---

## 10. Recoverable vs Unrecoverable Perturbation

One of the most important future decisions of the system is the distinction between:

* **recoverable perturbation**
* **unrecoverable perturbation**

This document does not yet define an exact formula.
But it must define the conceptual meaning.

### 10.1 Recoverable Perturbation

A perturbation is conceptually recoverable when the system still has a plausible path back to control through:

* current support
* reachable next support
* acceptable postural correction
* acceptable momentum reduction

### 10.2 Unrecoverable Perturbation

A perturbation is conceptually unrecoverable when the system no longer has a plausible path to restore controlled locomotion in time.

This may happen because:

* the CM is no longer capturable with available support
* momentum is too large
* the support geometry is no longer sufficient
* the body cannot reorganize fast enough

Later documents will turn this conceptual distinction into actual criteria.

---

## 11. Canonical Transition Logic

At the highest level, the project is organized around the following logic:

### 11.1 Nominal Branch

```text
procedural locomotion
    -> perturbation
    -> physical reaction
    -> recoverable
    -> recovery
    -> resynchronization
    -> procedural locomotion
```

### 11.2 Failure Branch

```text
procedural locomotion
    -> perturbation
    -> physical reaction
    -> unrecoverable
    -> fall
    -> grounded stabilization
    -> procedural get-up
    -> procedural locomotion
```

These two branches summarize the intended behavior of the whole hybrid architecture.

---

## 12. Regime Transition Principles

Even before exact formulas are introduced, some high-level transition rules can already be stated.

### 12.1 Transition Out of Nominal Procedural Motion

The character should leave the nominal procedural regime when:

* environment-driven motion becomes more important than planned motion
* the current disturbance cannot be represented credibly by nominal control alone

### 12.2 Transition from Physical Reaction to Recovery

The character should transition into recovery when:

* the perturbation remains physically significant
* but a plausible path back to control still exists

### 12.3 Transition from Physical Reaction to Falling

The character should transition into falling when:

* the perturbation is no longer capturable
* a support-based recovery is no longer plausible

### 12.4 Transition from Grounded Stabilization to Get-Up

The character should transition into procedural get-up when:

* it is sufficiently stabilized on the ground
* a coherent re-entry to intentional motion is possible

### 12.5 Transition from Get-Up to Nominal Procedural Motion

The character should transition back into nominal procedural control when:

* the body has reached a valid upright controllable state
* locomotion intent can be resumed

---

## 13. Regime Summary Table

| Regime | Main objective | Main CM interpretation |
|---|---|---|
| Nominal Procedural | intentional locomotion | controlled |
| Physical Reactive | credible dynamic reaction | increasingly emergent |
| Recovery | regain capturable control | re-constrained toward control |
| Falling | accept loss of balance | no longer capturable |
| Grounded Stabilization | settle after collapse | grounded and dissipating |
| Procedural Get-Up | return to intentional upright control | progressively controlled again |

This table is intentionally compact.
The text above should remain the main reference.

---

## 14. Terrain-General Interpretation of Regimes

It is important that the hybrid regime structure remain valid not only on simple flat terrain, but later also on:

* rough surfaces
* inclined surfaces
* uneven terrain
* discontinuous support geometry

### 14.1 Why This Matters Already

If the regime logic is written only for flat ground, future terrain extension may require redesigning the control structure itself.
That is exactly what the project should avoid.

### 14.2 Literature Support

[The Spring-Mass Model and Other Reductionist Models](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Spring-Mass%20Model%20and%20Other%20Reductionist%20Models.pdf) is especially relevant here because it explicitly extends reduced locomotion models to inclines and shows that the same conceptual family of models can remain meaningful on sloped ground.

[SIMBICON](/home/tomas/UL1/LIFAPCD/phy/doc/papers/2007-SIMBICON-Simple-Biped-Locomotion-Control.pdf) is also relevant because it explicitly demonstrates robustness to pushes, steps, slopes, and terrain variation.

### 14.3 Practical Consequence

The practical consequence for this project is:

* the **first implementation** may stay simple
* but the **regime structure** must already be terrain-extensible

That means the hybrid logic should always be formulated in terms of:

* support conditions
* capturability
* reachable support adaptation
* groundedness

and not only in terms of “standing on a horizontal floor.”

---

## 15. What Is Defined Here vs Later

### 15.1 Defined Here

This document defines:

* the main hybrid control regimes
* their conceptual purpose
* their relation to the CM
* their high-level transition logic
* the distinction between recoverable and unrecoverable perturbation
* the requirement that the regime structure remain extensible to future terrain complexity

### 15.2 Deferred to Later Documents

This document does **not** yet define:

* exact perturbation metrics
* exact switching thresholds
* exact PD control laws
* exact support planner logic
* exact resynchronization algorithms
* exact software module boundaries

Those elements belong to the next design layers.

---

## 16. Summary of Main Takeaways

1. The project requires a hybrid regime structure because neither pure procedural control nor pure physics is sufficient on its own.
2. The major regimes are: nominal procedural, physical reactive, recovery, falling, grounded stabilization, and procedural get-up.
3. Recovery and falling must be treated as explicit regimes, not as informal side effects.
4. The distinction between recoverable and unrecoverable perturbation is conceptually central, even if exact formulas are deferred.
5. The overall project logic follows two main branches:
   - recovery branch
   - failure and get-up branch
6. The entire regime structure should remain valid later on rough, sloped, uneven, or discontinuous terrain.

This document therefore establishes the **hybrid regime layer** of the project.
The next document should explain how the different control layers of the system are organized internally.
