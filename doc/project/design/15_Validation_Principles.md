# Validation Principles

## 1. Role of This Document

This document defines how the project should evaluate whether its behavior is:

* physically consistent
* biomechanically plausible within the adopted simplifications
* coherent across hybrid regimes
* visually credible enough for the intended result

The previous documents define what the system is supposed to do.
This document defines how the project should judge whether it is actually doing it well enough.

It exists because the project should not rely only on:

* intuition
* visual taste
* “it seems okay”

Validation must be part of the design, not only of the final demo.

---

## 2. What Validation Means in This Project

Validation in this project does **not** mean proving that the stickman is a complete biomechanical simulation of a human body.

That would be unrealistic and outside the scope of V1.

Instead, validation means checking that the system is:

* internally consistent
* physically non-absurd
* compatible with the reduced locomotion models it claims to use
* coherent with the support and recovery logic already defined
* visually plausible enough to communicate natural motion

So the project should aim for:

* **physical and biomechanical consistency within a simplified model**

rather than for:

* full medical or robotics-grade human realism

---

## 3. Why Validation Is Necessary

The project combines:

* CM-first nominal motion
* procedural reconstruction
* hybrid switching
* Box2D physical response
* recovery and falling
* stylized continuous-line rendering

Because of this, many failure modes can look acceptable at first glance while still being wrong in more objective terms.

Typical examples include:

* legs stretching beyond their reachable geometry
* support intervals that no longer match the visible feet
* recovery states classified as recoverable even though the next step is impossible
* jump flight that looks plausible in the air but lands with impossible support timing
* walking that looks animated but contradicts the intended CM logic

This is why validation must be defined explicitly.

---

## 4. Literature Basis

This document is guided by several papers already used in the dossier.

### 4.1 CoM as a System-Level Variable

[The Motion of Body Center of Mass During Walking: A Review Oriented to Clinical Applications.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf) supports the idea that walking should be judged at the level of whole-body transport, not only at the level of local limb motion.

This justifies validating:

* CM trajectory
* support transitions
* whole-body transport quality

### 4.2 CoM Trajectory Across Walking Speeds

[Trajectory of the human body mass centre during walking at different speeds.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Trajectory%20of%20the%20human%20body%20mass%20centre%20during%20walking%20at%20different%20speeds.pdf) reinforces that:

* the body center of mass is a key gait variable
* its trajectory changes with walking speed
* normal walking has variability bands rather than one exact rigid curve

This justifies validating trends and patterns, not only exact numerical matches.

### 4.3 Mechanical Distinction Between Walking and Running

[Mechanical work and efficiency in level walking and running.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Mechanical%20work%20and%20efficiency%20in%20level%20walking%20and%20running.pdf) highlights that walking and running are mechanically different, especially in:

* energy exchange
* external and internal work
* compliant versus more pendular behavior

This justifies validating walking and running as distinct regimes, not as one speed-scaled controller.

### 4.4 Robust State-Dependent Control

[Composite Control of Physically Simulated Characters.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Composite%20Control%20of%20Physically%20Simulated%20Characters.pdf) is relevant because it emphasizes:

* robustness under disturbance
* state-dependent response
* transition quality between control behaviors

This justifies validating not only nominal locomotion, but also:

* perturbation response
* recovery behavior
* state-driven transitions

### 4.5 Landing Quality

[Falling and Landing Motion Control for Character Animation.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Falling%20and%20Landing%20Motion%20Control%20for%20Character%20Animation.pdf) shows that good landing behavior should be judged through:

* impact management
* continuity after contact
* body organization after landing
* avoidance of unnecessary peak stress

This justifies a dedicated landing validation block.

---

## 5. Validation Philosophy for V1

For V1, validation should be:

* layered
* scenario-based
* partly quantitative
* partly qualitative

It should not depend on:

* large external datasets
* full motion-capture benchmarking
* expensive biomechanical instrumentation

Instead, V1 should validate behavior through five layers:

1. geometric consistency
2. kinematic consistency
3. dynamic and support consistency
4. regime-specific behavioral consistency
5. visual and interaction plausibility

---

## 6. Layer 1: Geometric Consistency

The first validation layer is purely geometric.

It checks whether the simulated and reconstructed body still respects the structure defined by the design documents.

### 6.1 Mandatory Geometric Checks

The system should verify:

* fixed segment lengths where expected
* no impossible leg reach beyond clamped geometry
* no impossible arm reach beyond clamped geometry
* consistency between visible feet and support geometry
* trunk and head continuity

### 6.2 Why This Layer Matters

A system that fails here is already invalid before any discussion of realism.

This layer is therefore the minimum validation floor.

---

## 7. Layer 2: Kinematic Consistency

The second validation layer checks whether the motion evolves coherently over time.

### 7.1 Mandatory Kinematic Checks

The system should verify:

* continuity of CM position
* continuity of CM velocity except at physically meaningful discontinuities
* continuity of limb motion without frame-to-frame flips
* knee branch consistency in analytic IK
* gait-phase coherence

### 7.2 Example of a Real Kinematic Failure

The system should flag if:

* the knee suddenly flips to the other IK branch
* the swing foot teleports
* the trunk lean changes discontinuously without a regime reason

### 7.3 Why This Layer Matters

Motion can be geometrically valid and still feel broken if temporal continuity is poor.

---

## 8. Layer 3: Dynamic and Support Consistency

This layer checks whether the motion respects the project’s physical and support logic.

### 8.1 Support Consistency

The system should validate:

* active support interval matches the current foot or feet in contact
* support changes are consistent with the current gait phase
* `Capture Point` classification is consistent with current support

### 8.2 Recovery Metric Consistency

The system should validate:

* `in-support recoverable` states keep `CP_x` inside the current `SupportInterval` for a short validation window without forced stepping
* `one-step recoverable` states actually admit a reachable recovery step that produces a new valid support interval
* `unrecoverable` states do not repeatedly fake upright recovery without establishing new support

### 8.3 Physical Consistency

When Box2D is authoritative, the system should validate:

* no severe contact explosions
* no obvious interpenetration with the ground
* no visibly impossible recovery from catastrophic states
* no repeated procedural/physical oscillation caused by unstable thresholds

### 8.4 Why This Layer Matters

This layer checks whether the hybrid logic is behaving as designed, not just whether the character looks animated.

---

## 9. Layer 4: Regime-Specific Validation

Different regimes must be validated differently.

The project should not use one universal criterion for everything.

### 9.1 Standing

Standing should be validated through:

* support stability
* bounded CM motion
* absence of unnecessary drift
* plausible postural micro-adjustment

### 9.2 Walking

Walking should be validated through:

* alternating support
* coherent swing/stance organization
* support changes that coincide with intended foot-contact events within a small simulation-step tolerance
* forward CM transport that remains monotonic within each stance interval
* moderate vertical oscillation that stays smaller than the running regime under the same tuning set
* absence of foot sliding during intended stance

The literature on CoM motion during walking suggests that whole-body transport should remain the main reading of the gait, not just local joint motion.

### 9.3 Running

Running should be validated through:

* a real flight phase
* stronger compliance than walking
* stronger trunk and arm contribution
* larger vertical oscillation or stance compression than walking

The project should explicitly reject running that is only “faster walking”.

### 9.4 Crouching

Crouching should be validated through:

* lower CM operating height
* stronger leg flexion
* shorter effective posture
* continuity with the standing and walking controllers

### 9.5 Jumping

Jumping should be validated through:

* loading phase before takeoff
* ballistic CM flight in nominal jump mode
* coherent transition from support to flight

### 9.6 Landing

Landing should be validated through:

* successful support reacquisition
* visible energy absorption
* continuity into the next state
* avoidance of instant stiff-stop behavior

Inspired by [Falling and Landing Motion Control for Character Animation.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Falling%20and%20Landing%20Motion%20Control%20for%20Character%20Animation.pdf), landing quality should also consider whether the body distributes impact credibly rather than collapsing unnaturally.

### 9.7 Recovery

Recovery should be validated through:

* correct classification
* plausible use of posture-only recovery for small disturbances
* use of one or more steps when appropriate
* actual return to a controllable state when recovery succeeds

### 9.8 Falling

Falling should be validated through:

* abandonment of fake nominal upright control
* physically plausible descent and contact
* continuity toward grounded stabilization

### 9.9 Get-Up

Get-up should be validated through:

* starting only from valid grounded states
* coherent progression toward upright posture
* no large snap at the entry or exit of the sequence

---

## 10. Layer 5: Visual Plausibility

Even if the motion is internally consistent, the final result must still read well on screen.

### 10.1 Visual Checks

The project should validate:

* silhouette readability
* continuity of the visible line body
* no accidental visual disconnection caused by reconstruction or rendering
* no support events that are hidden or contradicted by the drawn character

### 10.2 Why This Layer Matters

The project’s goal is not only simulation correctness.
It is also to produce a stickman that appears naturally animated as a continuous visible body.

So visual validation is legitimate, provided it comes after the more structural validation layers.

---

## 11. Quantitative vs Qualitative Validation

V1 should mix both.

### 11.1 Quantitative Validation

The project should use explicit measured quantities when possible, such as:

* CM height
* CM velocity
* support interval
* projected `Capture Point`
* stance duration
* number of recovery steps
* time to recover after perturbation

### 11.2 Qualitative Validation

The project should still keep qualitative review for:

* arm naturalness
* trunk expression
* fall believability
* get-up readability

### 11.3 Why Both Are Needed

Pure numbers can miss visual ugliness.
Pure visual review can miss structural errors.

The project therefore needs both.

---

## 12. Scenario-Based Validation

Validation should be driven by repeatable scenarios, not only by free exploration.

### 12.1 Core V1 Scenarios

At minimum, the project should test:

* stand still on flat ground
* walk on flat ground
* run on flat ground
* crouch from standing
* jump and land under nominal conditions
* small push during standing
* medium push requiring a recovery step
* large push causing a fall
* grounded stabilization after fall
* get-up from an accepted grounded pose

### 12.2 Why This Matters

Scenario-based testing lets the project compare:

* expected regime transitions
* expected support behavior
* expected visual outcome

against what actually happens.

---

## 13. Validation by Failure Detection

The project should also define what counts as an automatic failure.

### 13.1 Hard Failures

These should be treated as invalid behavior:

* impossible geometry
* uncontrolled body explosion
* clear ground interpenetration
* support logic contradicting visible contact state
* impossible recovery from an unrecoverable classification

### 13.2 Soft Failures

These are not immediate architecture failures, but should still be reported:

* excessive foot sliding
* overly stiff landing
* visually awkward arm behavior
* recovery that technically works but looks implausible

This distinction helps implementation priorities later.

---

## 14. Logging and Debug-Aided Validation

V1 validation should be supported by debug output.

At minimum, the project should be able to inspect:

* current regime
* current gait phase
* current support interval
* current `Capture Point`
* current recovery class
* whether the system is in procedural or physical authority

This is important because many hybrid failures are otherwise hard to interpret from visuals alone.

---

## 15. What V1 Should Not Try to Validate Yet

The following are outside the realistic validation scope of V1:

* exact anthropometric matching to human populations
* full energy accounting of all body segments
* medical-grade gait diagnosis
* detailed ground-reaction-force realism
* detailed heel-to-toe pressure progression

These may become future work, but they should not block V1.

---

## 16. Future Validation Extensions

A future version of the project could extend validation toward:

* richer biomechanical datasets
* more formal capture-region validation
* terrain-dependent validation
* better impact and landing metrics
* statistical comparison across many runs

V1 only needs a validation framework strong enough to support disciplined development.

---

## 17. Practical Use of This Document

This document should guide development in the following way:

* when a new feature is added, identify which validation layers it should satisfy
* when a bug appears, determine whether it is geometric, kinematic, support/dynamic, regime-specific, or visual
* when tuning parameters, prefer changes that improve multiple layers rather than only cosmetic appearance

This makes validation a development tool, not just a final review checklist.

---

## 18. Summary of Main Takeaways

* The project should validate consistency, plausibility, and regime behavior rather than claim full biomechanical realism.
* Validation should be layered: geometry, kinematics, dynamics/support, regime-specific behavior, and visual plausibility.
* Walking, running, jumping, landing, recovery, falling, and get-up each need distinct validation criteria.
* Scenario-based testing is essential for V1.
* Quantitative checks and qualitative review are both necessary.
* This document turns validation into a design responsibility rather than an afterthought.
