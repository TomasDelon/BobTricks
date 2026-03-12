# Recovery Metrics

## 1. Role of This Document

This document defines how the project decides whether a disturbed character state is:

* still controllable without changing support
* recoverable with a step
* recoverable with repeated stepping
* no longer recoverable, and therefore should transition into falling

The previous documents already defined:

* the CM-centered locomotion view
* the hybrid regime structure
* the control architecture
* the physical body model for V1
* the reconstruction and IK logic

What is still missing is a concrete answer to this question:

**when should the system try to recover, and when should it accept failure?**

This document answers that question for V1.

It does **not** aim to provide the most general capturability theory possible.
Instead, it defines a recovery metric that is:

* operational
* consistent with the literature
* suitable for a 2D biped
* achievable within the project time budget

---

## 2. Why Recovery Metrics Are Necessary

Without an explicit recovery metric, the hybrid architecture would have no principled way to decide:

* when to remain in nominal control
* when to enter recovery
* when a recovery step is needed
* when the character should stop trying to remain upright

This would make the transition logic arbitrary and hard to justify.

The literature makes it clear that this decision should be tied to:

* the state of the center of mass
* the current support region
* the possibility of changing support through stepping

This is the core idea behind:

* [Capture Point: A Step toward Humanoid Push Recovery.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Capture%20Point:%20A%20Step%20toward%20Humanoid%20Push%20Recovery.pdf)
* [Capturability-based Analysis and Control of Legged Locomotion, Part 1: Theory and Application to Three Simple Gait Models.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Capturability-based%20Analysis%20and%20Control%20of%20Legged%20Locomotion,%20Part%201:%20Theory%20and%20Application%20to%20Three%20Simple%20Gait%20Models.pdf)
* [Humanoid Push Recovery.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Humanoid%20Push%20Recovery.pdf)

So this document exists to turn those ideas into a V1 decision framework.

---

## 3. Literature Basis

The three most important scientific ideas behind this document are the following.

### 3.1 Capture Point

[Capture Point: A Step toward Humanoid Push Recovery.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Capture%20Point:%20A%20Step%20toward%20Humanoid%20Push%20Recovery.pdf) introduces a practical answer to:

* when a step is needed
* where a step should go
* when one step is not enough

Its key logic is:

* if the relevant capture quantity lies inside the current base of support, no step is required
* if it lies outside, a step is required
* if no feasible step can cover the capture region, recovery in one step fails

This is exactly the kind of reasoning needed here.

### 3.2 N-Step Capturability

[Capturability-based Analysis and Control of Legged Locomotion, Part 1: Theory and Application to Three Simple Gait Models.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Capturability-based%20Analysis%20and%20Control%20of%20Legged%20Locomotion,%20Part%201:%20Theory%20and%20Application%20to%20Three%20Simple%20Gait%20Models.pdf) extends the previous idea by asking:

* can the system stop without falling in one step?
* in two steps?
* in N steps?

This is important because “recoverable” is not binary in practice.
Some disturbances require:

* no step
* one step
* several steps

This layered interpretation is very useful for the project.

### 3.3 Ankle, Hip, and Step Strategies

[Humanoid Push Recovery.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Humanoid%20Push%20Recovery.pdf) organizes push recovery into three broad strategy families:

* support modulation through the current support
* internal body reorganization
* stepping

This fits very well with the hybrid regimes already defined in the dossier.

---

## 4. V1 Philosophy

V1 will not attempt to implement a mathematically complete general capturability solver.

Instead, V1 will use a **capture-inspired recovery metric** that combines:

* current CM state
* current support region
* reachable next support locations
* simple regime-specific thresholds

This choice is deliberate.

The project needs a criterion that is:

* operational in 2D
* compatible with finite feet
* understandable
* good enough to drive regime switching and stepping decisions

So V1 uses the literature as a conceptual guide, but keeps the implementation simpler than the full general theory.

---

## 5. Main Quantities Used by the Metric

For V1, the recovery metric should reason with the following quantities.

### 5.1 CM Position and Velocity

The metric must consider:

* current CM position
* current CM velocity

Recovery cannot be judged from position alone.
Velocity is essential, which is one of the main lessons of capture-point and push-recovery literature.

### 5.2 Current Support Region

The metric must know:

* which foot or feet are in support
* the current support interval in the locomotion plane
* whether support is single, double, or absent

In V1, this support region should be based on the finite foot chosen in [09_Physical_Body_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/09_Physical_Body_Model.md), not on a purely punctual contact.

For V1, the natural 2D representation is:

```text
SupportInterval = [xMin, xMax]
```

or equivalently:

```text
SupportInterval = (center, halfWidth)
```

In single support, this interval is the horizontal support extent of the active foot.
In double support, it is the interval spanning the combined support of both feet.

### 5.3 Reachable Next Support Region

The metric must also know where the swing foot could reasonably land next.

This includes:

* step direction
* step length limit
* simple geometric reachability
* terrain validity of the candidate foothold

### 5.4 Recovery Margin

The metric should not rely only on exact equality such as “inside” versus “outside”.
It should also use a safety margin around support limits.

This is important because support near the edge is already fragile, even if it is not yet formally lost.

### 5.5 Capture Point Quantity for V1

For V1, the main projected capture quantity should be the 2D horizontal Capture Point derived from the linear inverted-pendulum baseline:

```text
omega0 = sqrt(g / h_cm_ref)
CP_x   = x_cm + v_cm / omega0
```

where:

* `x_cm` is the current horizontal CM position in the locomotion plane
* `v_cm` is the current horizontal CM velocity
* `h_cm_ref` is the current reference CM operating height for the active regime

This is the main capture-inspired scalar that the V1 recovery metric should compare against current and reachable support intervals.

---

## 6. Recovery Classes for V1

For V1, disturbed states should be classified into four main classes.

### 6.1 In-Support Recoverable

A state is **in-support recoverable** if the character can plausibly recover without changing the support foot.

Conceptually, this corresponds to:

* current CM behavior still capturable by the current support
* no urgent recovery step required
* posture or support modulation may be enough

This class corresponds roughly to the smaller-disturbance range of ankle or posture-based recovery.

### 6.2 One-Step Recoverable

A state is **one-step recoverable** if the current support is no longer sufficient, but a single reachable step can recover control.

Conceptually, this means:

* the current support region no longer contains the needed capture behavior
* but a reachable next support region can

This is the most important recovery class for V1.

### 6.3 Repeated-Step Recoverable

A state is **repeated-step recoverable** if it is not safely recoverable in the current support and not clearly guaranteed in one perfect step, but the system can still plausibly try to regain control through a sequence of reactive recovery steps.

For V1, this class should be treated in a simplified way:

* not as full explicit N-step planning
* but as repeated one-step re-evaluation after each recovery step

This is a practical approximation of the N-step capturability logic from the literature.

### 6.4 Unrecoverable

A state is **unrecoverable** if the current support cannot recover it and no plausible next support action remains within the V1 limits.

At that point, the system should stop pretending that upright recovery is still available and should transition to falling.

---

## 7. Capture-Inspired Decision Principle

The guiding rule for V1 should be:

* if the required capture behavior lies inside the current support, remain recoverable without stepping
* if it lies outside the current support but inside a reachable next support region, use a recovery step
* if it lies outside both, the state is unrecoverable for V1

In V1, the phrase “required capture behavior” should be understood primarily through the projected quantity `CP_x`.

This is directly inspired by the logic explained in [Capture Point: A Step toward Humanoid Push Recovery.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Capture%20Point:%20A%20Step%20toward%20Humanoid%20Push%20Recovery.pdf).

The important point is that recovery is judged relative to:

* support
* foot reachability
* current motion of the CM

not relative to posture appearance alone.

---

## 8. A V1 Operational Metric

For V1, the most useful metric is not a full closed-form capturability proof.
It is an operational decision process.

### 8.1 Step 1: Evaluate Current Support Sufficiency

The system first asks:

* is the current support region still sufficient for the current CM state?

This test can combine:

* CM position relative to support
* CM velocity magnitude and direction
* the projected capture point `CP_x`
* support margin

If this test succeeds, the state is classified as **in-support recoverable**.

### 8.2 Step 2: Evaluate One-Step Recovery

If the first test fails, the system asks:

* is there a reachable next foothold whose support interval would contain `CP_x` with margin?

If yes, the state is classified as **one-step recoverable**.

### 8.3 Step 3: Evaluate Repeated-Step Attempt

If no clean one-step recovery is available, the system may still classify the state as **repeated-step recoverable** if:

* the body is still upright enough
* the disturbance is not obviously catastrophic
* a reactive stepping attempt is still plausible
* the projected capture point is still moving toward a plausible future support solution rather than diverging farther away after each recovery step

In V1 this should remain conservative.
This class exists to allow several reactive steps without claiming a formal N-step planner.

### 8.4 Step 4: Declare Failure

If the previous tests fail, the state becomes **unrecoverable**.

At that point the regime manager should allow:

* transition to falling
* abandonment of upright nominal posture goals
* preparation for grounded stabilization

---

## 9. Recoverability Without Stepping

The simplest recovery case is when the character does not need a new step.

This case should depend on three ideas:

### 9.1 Support Containment

The projected capture point `CP_x` must remain inside the current support interval with margin.

### 9.2 Velocity Bound

The CM velocity must remain small enough that the body can plausibly dissipate or redirect it through current support and posture.

### 9.3 Posture Feasibility

The reconstruction layer must still be able to build a plausible stance posture without violating limits or producing impossible leg geometry.

So in-support recovery is not only a CM test.
It is also a support-plus-posture feasibility test.

---

## 10. One-Step Recovery

One-step recovery is the central reactive behavior for V1.

### 10.1 Core Criterion

A state should be considered one-step recoverable if:

* the current support is insufficient
* a new support can be reached in time
* the new support interval would contain `CP_x` with margin

### 10.2 What “Reachable” Means in V1

For V1, reachability should be defined conservatively using:

* maximum step length
* allowed step direction
* current swing-foot availability
* simple terrain validity

This keeps the criterion compatible with the physical and geometric body model.

### 10.3 Why This Is Enough for V1

The literature contains much richer treatments of stepping recovery and N-step reasoning.
However, one-step recovery already captures the most visible and useful reactive behavior:

* being pushed
* taking a corrective step
* regaining balance

So one-step recoverability should be treated as a first-class V1 target.

---

## 11. Repeated-Step Recovery

The literature shows that some systems are not one-step capturable but are still recoverable in more than one step.

This is one of the important contributions of [Capturability-based Analysis and Control of Legged Locomotion, Part 1: Theory and Application to Three Simple Gait Models.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Capturability-based%20Analysis%20and%20Control%20of%20Legged%20Locomotion,%20Part%201:%20Theory%20and%20Application%20to%20Three%20Simple%20Gait%20Models.pdf).

For V1, however, repeated-step recovery should be approximated simply:

* attempt one reactive recovery step
* re-evaluate the disturbed state after that step
* if the new `CP_x` is closer to a plausible next support interval and the body remains sufficiently upright, allow another recovery step
* if the state worsens beyond the allowed thresholds, transition to falling

So V1 acknowledges multi-step recovery conceptually, but implements it as **iterated one-step recovery** rather than as a full N-step capturability solver.

---

## 12. Unrecoverable States

The project must explicitly recognize that some disturbed states should no longer be treated as balance recovery problems.

Typical causes include:

* required capture support lies beyond reachable step range
* support is lost while the body is already too collapsed
* the body is rotating or descending too violently
* no valid foothold is available
* the reconstruction needed for recovery is no longer feasible

When these conditions hold, the correct system response is not “try harder”.
It is to transition into falling.

This is important both physically and visually.

---

## 13. Support Geometry Matters

The recovery metric depends strongly on the foot model.

This is one of the important messages of the capturability literature:

* a point foot and a finite foot do not offer the same capturability
* internal angular momentum and finite support enlarge recovery options

This is one of the reasons why [09_Physical_Body_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/09_Physical_Body_Model.md) selected a finite simple foot for V1 rather than a purely point-foot support model.

So the recovery metric should always be interpreted together with:

* support width
* step reachability
* available contact geometry

---

## 14. Relation to the Hybrid Regimes

The recovery metric is the missing bridge between support reasoning and regime switching.

Its role is:

* **in-support recoverable** -> remain in or near nominal control, possibly with corrective posture
* **one-step recoverable** -> enter recovery with explicit reactive step
* **repeated-step recoverable** -> remain in recovery, re-evaluating after each step
* **unrecoverable** -> enter falling

This makes the regime transitions explainable rather than purely heuristic.

---

## 15. Why V1 Does Not Use a Full Formal Capturability Solver

The literature contains richer frameworks than V1 will implement.

These include:

* explicit N-step capture regions
* more complete dynamic models
* richer use of internal angular momentum
* optimization-based controllers

Those are valuable references, and they justify the conceptual direction of the project.

However, for V1 they would add too much implementation complexity.

So the V1 design choice is:

* use capture-point and capturability ideas as the conceptual basis
* implement a simpler operational decision metric
* keep the architecture compatible with future extension

This is an intentional engineering compromise, not a misunderstanding of the literature.

---

## 16. Future Evolution

A future version of the project could enrich recovery metrics with:

* more explicit capture-point computation
* formal one-step capture-region geometry
* explicit N-step capturability estimation
* support adaptation on uneven or discontinuous terrain
* richer use of angular momentum in recovery

This document only fixes the V1 baseline.

---

## 17. What Is Deferred to Later Documents

This document does not yet define:

* exact threshold values
* exact maximum step lengths
* exact timing windows for stepping
* the final update-order integration of these tests
* quantitative validation of recovery quality

Those elements belong to later design and validation documents.

---

## 18. Summary of Main Takeaways

* Recovery must be decided from CM state, support, and step reachability, not from pose appearance alone.
* V1 uses a capture-inspired operational metric rather than a full formal capturability solver.
* Disturbed states are classified as in-support recoverable, one-step recoverable, repeated-step recoverable, or unrecoverable.
* One-step recovery is the most important explicit V1 recovery behavior.
* Repeated-step recovery is approximated in V1 through repeated one-step re-evaluation.
* Unrecoverable states should transition to falling rather than forcing a fake recovery.
