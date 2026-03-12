# Reconstruction and IK

## 1. Role of This Document

This document defines how the project reconstructs a **full articulated target pose** from higher-level locomotion objectives.

The previous documents already defined:

* the abstract stickman structure
* the CM-centered locomotion logic
* the hybrid control regimes
* the control architecture
* the physical body model used in Box2D

What is still missing is the bridge between:

* global targets such as CM evolution and support targets
* local targets such as joint angles, limb configurations, and trunk orientation

This document therefore answers the following questions:

* How is a procedural pose reconstructed from CM and support information?
* What kind of IK strategy is used in V1?
* Which tasks have priority over others?
* How do legs, trunk, head, and arms get their targets?
* What happens when a target is unreachable?

This document does **not** yet define:

* the formal recovery metric
* the exact update loop
* the final module APIs
* the exact torque or motor law used in the physical regime

Those elements belong to later documents.

---

## 2. Why Reconstruction Is Necessary

The project is not driven directly by keyframed joint angles.

Instead, the control stack first reasons about:

* locomotion intent
* hybrid regime
* CM behavior
* support and foot placement

This means that the system still needs a layer that converts those global decisions into a **concrete articulated posture**.

Without such a layer:

* the CM controller would produce no visible body
* the foot placement planner would produce targets with no leg solution
* the physical tracking layer would have no procedural pose to follow

So reconstruction is not optional.
It is the central bridge between global locomotion logic and articulated body behavior.

---

## 3. Reconstruction Philosophy for V1

V1 will not use a full general-purpose whole-body optimizer at every frame.

Instead, V1 will use a **hybrid reconstruction strategy**:

* a **task-priority philosophy** inspired by [An Inverse Kinematic Architecture Enforcing an Arbitrary Number of Strict Priority Levels.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/An%20Inverse%20Kinematic%20Architecture%20Enforcing%20an%20Arbitrary%20Number%20of%20Strict%20Priority%20Levels.pdf)
* simple **analytic 2D IK** for two-link limbs whenever a closed-form solution is natural
* explicit geometric and contact constraints, in the spirit of [Inverse Kinematics and Geometric Constraints for Articulated Figure Manipulation.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Inverse%20Kinematics%20and%20Geometric%20Constraints%20for%20Articulated%20Figure%20Manipulation.pdf)
* regime-dependent posture heuristics for trunk, arms, and preferred stance

This choice is deliberate.

For V1, the goal is not to implement the most general IK architecture.
The goal is to define a reconstruction process that is:

* understandable
* controllable
* compatible with a 2D stickman
* realistic enough for locomotion and perturbation scenarios
* achievable within the time budget

This also matches the practical spirit of [Animation Bootcamp - An Indie Approach to Procedural Animation.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Animation%20Bootcamp%20-%20An%20Indie%20Approach%20to%20Procedural%20Animation.pdf), where simple procedural building blocks, IK, springs, and physics are combined rather than replaced by a single monolithic solver.

---

## 4. Inputs to Reconstruction

The reconstruction layer does not invent its own goals.
It receives them from the higher layers of the control architecture.

Its main conceptual inputs are:

* the current **hybrid regime**
* the current or target **CM state**
* the current or target **support configuration**
* the current or target **foot placement**
* the current **phase of locomotion**
* a preferred **posture style** for the active regime

In V1, reconstruction should also know:

* which foot is in stance
* which foot is in swing
* whether a foot is constrained by contact
* whether the character is airborne
* whether the character is grounded after a fall

This makes reconstruction a **constraint-aware** layer rather than a purely decorative pose generator.

---

## 5. Outputs of Reconstruction

The reconstruction layer produces a **procedural articulated target state**.

Its outputs conceptually include:

* target joint angles
* target node positions
* target trunk orientation
* target head orientation or head attachment position
* target leg configurations
* target arm configurations

In V1, the most important result is a pose target that can later be:

* rendered directly in nominal procedural motion
* used as a reference when synchronizing back from physical motion
* reused by the physical tracking layer in future richer architectures

### 5.1 Interface from the CM Controller

For V1, the reconstruction layer should not receive only an abstract “CM idea”.
It should receive a minimal geometric interface from the CM controller.

That interface should conceptually include:

* `cmTargetPosition`
* `cmTargetVelocity`
* `pelvisOffsetTarget`
* `trunkLeanTarget`
* `airborneState`

During nominal jump flight, this interface should also include a CM target that already follows ballistic evolution.

In other words:

* the **CM controller** is responsible for producing the global CM trajectory
* the **reconstruction layer** is responsible for organizing the articulated body around that trajectory

---

## 6. Task Priority Structure

The key idea of this document is that reconstruction should not solve all goals at the same level.

This follows the logic of strict-priority IK and whole-body task organization seen in:

* [An Inverse Kinematic Architecture Enforcing an Arbitrary Number of Strict Priority Levels.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/An%20Inverse%20Kinematic%20Architecture%20Enforcing%20an%20Arbitrary%20Number%20of%20Strict%20Priority%20Levels.pdf)
* [Whole-body Motion Integrating the Capture Point in the Operational-Space Inverse Dynamics Control Framework.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Whole-body%20Motion%20Integrating%20the%20Capture%20Point%20in%20the%20Operational-Space%20Inverse%20Dynamics%20Control%20Framework.pdf)

For V1, the conceptual priority stack should be:

1. **support and contact constraints**
2. **leg reachability for the active support state**
3. **trunk placement and orientation consistent with the CM objective**
4. **swing-limb goals**
5. **posture preference and style**
6. **arm shaping and secondary visual behavior**

This means, for example:

* a planted foot should not be moved just to improve arm symmetry
* a reachable stance leg should not be broken just to keep the torso perfectly upright
* posture style should yield when contact and support require it

This priority structure is one of the most important design decisions of the reconstruction system.

---

## 7. Leg Reconstruction in 2D

For V1, each leg should be reconstructed as a **planar two-link chain**.

This is the simplest place where analytic IK is both natural and useful.

### 7.1 Leg Chain Definition

Each leg is modeled as:

* hip origin
* upper leg
* lower leg
* ankle or foot target

The segment lengths are fixed by the body model and the nominal stickman geometry.

### 7.2 Why Analytic IK Is Preferred for Legs

In a planar two-link chain:

* the target ankle position is known or chosen
* the hip position is known from the torso reconstruction
* the knee position can be solved geometrically

This makes a full iterative IK solver unnecessary for the basic leg problem.

For V1, this is a strong simplification because it gives:

* predictable behavior
* low implementation complexity
* clear reachability conditions

### 7.3 Closed-Form Two-Link Leg IK

For V1, the leg IK should be written explicitly in geometric form.

Let:

* `h = hip position`
* `a = ankle target position`
* `L1 = upper-leg length`
* `L2 = lower-leg length`
* `d = |a - h|`

Then the basic planar solution is:

```text
d      = clamp(|a - h|, epsilon, L1 + L2)
theta  = atan2(a.y - h.y, a.x - h.x)
alpha  = acos(clamp((L1^2 + d^2 - L2^2) / (2 * L1 * d), -1, 1))
beta   = acos(clamp((L1^2 + L2^2 - d^2) / (2 * L1 * L2), -1, 1))
```

The two geometric knee candidates are:

```text
knee(h, +) = h + L1 * (cos(theta + alpha), sin(theta + alpha))
knee(h, -) = h + L1 * (cos(theta - alpha), sin(theta - alpha))
```

The corresponding local angles are then obtained from the chosen branch.

For example:

```text
hipAngle  = theta +/- alpha
kneeAngle = pi - beta
```

This is the closed-form two-link solution that V1 should implement for the baseline leg solver.

### 7.4 Stance Leg vs Swing Leg

The two legs do not play the same role.

The **stance leg** must mainly satisfy:

* contact consistency
* support geometry
* trunk support
* compatibility with the current CM target

The **swing leg** must mainly satisfy:

* clearance from the ground
* movement toward the next support target
* a plausible knee bend direction

So the leg solver is not just “the same IK twice”.
It is the same kinematic tool used under different task priorities.

### 7.5 Knee Bend Convention

The reconstruction must define a consistent bend direction for the knee.

In V1:

* the knee should bend forward in the chosen locomotion plane
* the solver should avoid branch ambiguity where the knee flips unnaturally

In the closed-form notation above, this means the solver must consistently choose one of the two branches:

* `theta + alpha`
* `theta - alpha`

according to the selected forward-bending convention.

If two geometric solutions exist, the chosen one should be the one closest to:

* the previous frame
* the expected phase of gait
* the current locomotion regime

This temporal consistency matters as much as geometric correctness.

### 7.6 Reachability and Leg Compression

If a requested ankle target is:

* fully reachable, the leg uses its normal IK solution
* slightly too close, the leg may compress within joint limits
* too far, the target must be clamped or the higher-level planner must be corrected

Reconstruction must therefore never pretend that all support targets are feasible.
It must explicitly detect infeasible targets.

---

## 8. Trunk and Head Reconstruction

The trunk is not solved the same way as the legs.

For V1, trunk reconstruction should be based on:

* the support state
* the target CM operating point
* regime-dependent orientation rules

### 8.1 Trunk Position

The lower trunk or pelvis region defines the effective origin of the leg chains.

So trunk reconstruction must first determine:

* pelvis or lower-torso position
* upper-torso orientation
* the resulting hip locations

This is the main geometric bridge between CM logic and leg IK.

For V1, this bridge should be made explicit:

```text
pelvisTarget = cmTargetPosition + pelvisOffsetTarget
```

This means that the CM controller must provide not only a CM trajectory, but also the pelvis offset that allows reconstruction to place the trunk consistently.

### 8.1.1 V1 Pelvis Offset by Regime

The baseline V1 relation should be:

* **standing**: `pelvisOffsetTarget = (0, -0.85 * L_rest)`
* **walking**: `pelvisOffsetTarget = (x_walk_phase, -0.85 * L_rest)`
* **running**: `pelvisOffsetTarget = (x_run_bias, -h_run_offset)`

where:

* `x_walk_phase` is a small phase-dependent horizontal bias toward the current stance organization
* `x_run_bias` is a small rearward bias relative to the CM, compatible with stronger forward trunk lean
* `h_run_offset` is larger than the standing offset during stance compression and may relax again during aerial flight

So V1 does not assume a single immutable CM-to-pelvis relation for every locomotion regime.
It uses a simple regime-dependent offset model instead.

### 8.2 Trunk Orientation by Regime

The trunk should not remain rigidly vertical in every regime.

For V1, a reasonable regime-dependent policy is:

* **standing**: near-upright
* **walking**: mostly upright with mild anticipatory lean
* **running**: clearer forward lean
* **crouching**: lowered and slightly folded
* **jump preparation**: stronger crouched loading posture
* **landing**: temporary forward and downward absorption posture
* **recovery**: oriented to help recapture support
* **falling**: no longer forced toward nominal upright posture
* **get-up**: follows a dedicated procedural sequence

This means trunk reconstruction is not a passive afterthought.
It is part of how the system communicates effort, balance, and intent.

### 8.3 Head Reconstruction

In V1, the head can be reconstructed simply:

* attached to the top of the trunk
* aligned with the trunk or with a small independent offset

The head is not a high-priority balance effector in V1.
Its main role is structural legibility and visual continuity.

---

## 9. Arm Reconstruction in 2D

For V1, each arm should also be treated as a planar two-link chain.

However, arm reconstruction is lower priority than leg and trunk reconstruction.

### 9.1 Why Arms Are Secondary in V1

The arms are important visually, but they are not the primary source of support in the baseline locomotion of V1.

So they should not dominate the solution when:

* contact constraints are active
* the trunk must reposition for support
* the legs need a feasible stance or swing solution

### 9.2 Arm Behavior by Regime

A practical V1 policy is:

* **standing**: relaxed, low-amplitude balancing posture
* **walking**: simple rhythmic counter-swing
* **running**: stronger counter-swing
* **crouching**: folded closer to the body
* **jump preparation**: anticipatory swing or loading posture
* **landing**: balancing or protective pose
* **recovery**: rebalancing aid or stabilizing posture
* **falling**: protective or passive response
* **get-up**: regime-specific scripted procedural support role

This is intentionally simpler than leg reconstruction.

### 9.3 Analytic IK for Arms

When a specific arm endpoint target is known, the arm can use the same two-link analytic logic as the leg.

When no strict endpoint target exists, the arm can instead be set by:

* posture rules
* phase rules
* simple oscillation rules

So V1 arm reconstruction should be **hybrid**:

* IK when needed
* posture heuristics when endpoint precision is unnecessary

---

## 10. Reconstruction by Regime

The reconstruction layer does not use a single posture recipe for all behaviors.

Its priorities and posture heuristics change with the active regime.

### 10.1 Standing

Standing reconstruction should prioritize:

* stable support
* near-upright trunk
* nominal CM operating height
* low-amplitude balancing adjustments

### 10.2 Walking

Walking reconstruction should prioritize:

* stance-foot consistency
* swing-foot progression
* mild trunk transport over the support leg
* rhythmic arm counter-swing

This regime should remain visually simple and readable in V1.

### 10.3 Running

Running reconstruction should prioritize:

* stronger trunk lean
* larger leg compression and extension
* clearer aerial phase behavior
* stronger arm participation

Running should not be reconstructed as “walking with faster feet”.

### 10.4 Crouching

Crouching reconstruction should prioritize:

* lowered trunk and CM operating height
* increased flexion in legs
* reduced limb extension

### 10.5 Jump Preparation, Flight, and Landing

These phases require different posture goals:

* **preparation**: lowering and loading
* **flight**: no support contact, ballistic body organization
* **landing**: rapid support reacquisition and energy absorption

Landing reconstruction should give clear priority to support and absorption over visual symmetry.

During nominal jump flight, the ballistic evolution of the CM is not generated by the reconstruction layer itself.
It must come from the CM controller, which should integrate the airborne CM target over time and provide it to reconstruction as part of its normal interface.

### 10.6 Recovery

Recovery reconstruction should prioritize:

* support preservation or reacquisition
* feasible step targets
* trunk reorientation for recapture
* suppression of unnecessary visual styling

This is the regime where task priority matters most clearly.

### 10.7 Falling

During falling, reconstruction should stop insisting on nominal locomotion posture.

Instead, it should allow:

* loss of upright posture
* protective arm behavior if desired
* physically plausible body organization

### 10.8 Get-Up

Get-up should not be solved by the same generic locomotion reconstruction used in standing or walking.

For V1, get-up should be treated as a dedicated procedural sequence whose internal poses are reconstructed more explicitly and more simply.

---

## 11. Reachability, Clamping, and Infeasible Targets

Reconstruction must explicitly acknowledge that some higher-level targets may be impossible.

Examples include:

* a foot target that lies beyond leg reach
* a trunk position incompatible with the active support
* a posture request that violates joint limits

In such cases, V1 should prefer:

* clamping the target to a feasible range
* preserving contact and support
* reporting the infeasibility to the higher-level logic if needed

It should **not** silently generate impossible postures.

This is also one of the practical lessons behind task-priority IK and contact-aware whole-body control:
not every desired task can be satisfied simultaneously.

---

## 12. Relation to the Physical Body Model

The reconstruction defined here produces a **procedural articulated target**.

It does not replace the physical body defined in [09_Physical_Body_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/09_Physical_Body_Model.md).

In V1:

* nominal motion uses the reconstructed procedural pose directly
* strongly reactive phases rely on the physical body in Box2D
* later resynchronization may use the reconstruction layer again to rebuild a procedural target from the recovered state

So reconstruction belongs mainly to the procedural side of the hybrid architecture, even though it must remain compatible with the physical side.

---

## 13. Why V1 Does Not Use Full Whole-Body Optimization

The literature contains richer formulations based on:

* strict task-priority IK
* hierarchical quadratic programming
* operational-space inverse dynamics

Examples include:

* [An Inverse Kinematic Architecture Enforcing an Arbitrary Number of Strict Priority Levels.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/An%20Inverse%20Kinematic%20Architecture%20Enforcing%20an%20Arbitrary%20Number%20of%20Strict%20Priority%20Levels.pdf)
* [Whole-body Motion Integrating the Capture Point in the Operational-Space Inverse Dynamics Control Framework.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Whole-body%20Motion%20Integrating%20the%20Capture%20Point%20in%20the%20Operational-Space%20Inverse%20Dynamics%20Control%20Framework.pdf)

These approaches are valuable and should influence the design philosophy of the project.

However, V1 does not need to implement a full whole-body optimizer.

For this project stage, a simpler architecture is more reasonable:

* explicit priorities
* analytic two-link IK where possible
* regime-dependent heuristics
* geometric feasibility checks

This keeps the reconstruction system understandable and achievable while staying compatible with future growth.

---

## 14. Future Evolution

A future version of the project could evolve this V1 reconstruction toward:

* stricter task-priority enforcement
* iterative or optimization-based whole-body IK
* richer contact reasoning
* explicit heel/toe support handling
* better arm use during balance and recovery
* tighter coupling with physical tracking

So V1 is not the final architecture.
It is the minimal reconstruction design that can support the project goals without overextending implementation complexity.

---

## 15. What Is Deferred to Later Documents

This document does not yet define:

* the formal recoverability test
* the final gait phase representation
* the exact update ordering
* the exact interface between reconstruction and modules
* the quantitative validation of reconstructed poses

These topics belong to later design documents.

---

## 16. Summary of Main Takeaways

* Reconstruction is the bridge between CM/support logic and articulated posture.
* V1 uses a hybrid strategy: task-priority reasoning plus analytic 2D IK where possible.
* Support and contact constraints have priority over aesthetic posture goals.
* Legs are reconstructed mainly through two-link planar IK with stance/swing-specific roles.
* The trunk is reconstructed from support, CM logic, and regime-dependent posture rules.
* Arms are lower priority and may combine IK with simpler posture heuristics.
* V1 deliberately avoids a full whole-body optimization framework, while remaining compatible with future evolution toward one.
