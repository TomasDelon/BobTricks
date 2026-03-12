# Rendering Model

## 1. Role of This Document

This document defines how the internal stickman state should be turned into the **final visible character**.

The previous documents established:

* the project vision
* the internal stickman model
* the CM-centered locomotion logic
* the hybrid control regimes
* the control architecture
* the continuity requirements between regimes

What is still missing is the visual representation layer:

* how the articulated internal model should appear on screen
* how the body should look like one continuous line instead of disconnected rigid bars
* how rendering should remain compatible with SDL and MVC

This document therefore answers the following questions:

* What is the visual goal of the character?
* How is the internal skeleton mapped to a rendered shape?
* Why are splines or smooth curves needed?
* Which rendering quantities belong to the View rather than to the Model?
* How should the rendering layer remain compatible with future terrain and contact complexity?

This document does **not** yet define:

* exact SDL drawing code
* exact spline formulas
* exact anti-aliasing technique
* exact performance optimizations
* exact software classes

Those topics belong to later technical documents.

---

## 2. Why a Rendering Model Is Needed

The project does not want the final character to look like a debugging skeleton.

Internally, the stickman is represented by:

* a small set of nodes
* a simple articulated graph
* support-relevant endpoints
* control-oriented geometric quantities

That is useful for simulation and locomotion design, but not sufficient for the final appearance.

The visible character should instead read as:

* one coherent body
* one continuous graphical identity
* one line-like animated figure rather than a collection of disconnected segments

This is why a dedicated rendering model is necessary.

---

## 3. Visual Goal

The final character should appear as a **continuous line-based stickman**.

This goal has several consequences.

### 3.1 One Body, Not a Bag of Segments

The rendered figure should not look like:

* isolated rods
* disconnected circles
* a rigid technical skeleton

Instead, the viewer should read:

* a continuous body line
* smooth body flow from trunk to limbs
* a coherent silhouette even in motion

### 3.2 Readability First

The rendering style should remain easy to read during:

* standing
* walking
* running
* jumping
* recovery
* falling
* get-up

This means the line quality must support motion comprehension, not only style.

### 3.3 Continuity Must Survive Regime Changes

Because the system changes between procedural and physical regimes, the rendering must not make those transitions look like character swaps.

The same visual identity should survive:

* perturbation
* recovery steps
* collapse
* grounded stabilization
* procedural re-entry

---

## 4. Internal Structure vs Rendered Appearance

One of the most important distinctions in the entire project is the difference between:

* the **internal articulated model**
* the **external rendered body**

### 4.1 The Internal Model Is Control-Oriented

The internal model exists to support:

* locomotion logic
* support reasoning
* CM organization
* articulated reconstruction
* physical tracking

It is therefore deliberately minimal.

### 4.2 The Rendered Model Is Appearance-Oriented

The rendered model exists to support:

* visual continuity
* body readability
* aesthetic coherence
* screen-space presentation

It does not have to expose the raw internal graph directly.

### 4.3 Why This Separation Matters

If the internal skeleton were rendered directly as-is, the final result would likely look:

* too rigid
* too technical
* too disconnected
* too close to a debugging overlay

The rendering model therefore acts as a **visual interpretation layer** placed on top of the internal body state.

---

## 5. The Rendering Pipeline in Conceptual Terms

At the conceptual level, the rendering process should work like this:

1. the simulation produces the current stickman state
2. the View extracts the current visible node positions
3. those positions are converted into rendering control points
4. smooth curves or spline segments are generated from those points
5. the final stroke-like figure is rasterized in SDL

This is not yet a software class diagram.
It is the conceptual rendering flow.

---

## 6. Visible Anchors and Rendering Control Points

The internal nodes of the stickman are not automatically the same thing as the final spline control points, but they are the main source from which those control points will be derived.

### 6.1 Primary Visible Anchors

The following body nodes are the main visible anchors:

* `head_top`
* `torso_top`
* `torso_center`
* `torso_bottom`
* `elbow_left`
* `wrist_left`
* `elbow_right`
* `wrist_right`
* `knee_left`
* `ankle_left`
* `knee_right`
* `ankle_right`

These anchors come from [01_Stickman_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/01_Stickman_Model.md).

### 6.2 Rendering Control Points May Be Richer Than the Body Graph

The rendering layer may later introduce additional control points that do not exist as physical or articulated joints.

Examples:

* points used to round the trunk curve
* points used to shape arm curvature
* points used to join limb branches more smoothly
* points used to stabilize head attachment visually

This is acceptable because rendering control points are not body nodes.
They are visual helpers.

---

## 7. Why Smooth Curves Are Needed

If the body were drawn only with straight segments between raw nodes, the final result would remain too close to the internal skeleton.

Smooth curves are needed for at least four reasons.

### 7.1 Visual Coherence

Curves help the figure read as one continuous body rather than a set of independent sticks.

### 7.2 Regime Robustness

When the character changes posture abruptly because of perturbation or recovery, curves help maintain a coherent visual body even if the internal articulation changes quickly.

### 7.3 Better Trunk Representation

The trunk is internally only a short vertical chain.
A curve lets it appear as a unified central body line rather than as stacked rigid segments.

### 7.4 Better Limb Integration

Curves can visually soften the joins between:

* torso and arms
* torso and legs
* head and torso

This is especially important because the internal model intentionally omits explicit shoulders and hips.

---

## 8. Curve-Based Representation Strategy

The project should think of the visible character as a set of connected curve branches rather than as raw bone segments.

### 8.1 Main Visual Components

A practical rendering decomposition is:

* one central trunk/head line
* one left arm branch
* one right arm branch
* one left leg branch
* one right leg branch

These are not separate characters.
They are connected visual branches of the same body.

### 8.2 Central Body Line

The central body line should visually organize:

* `head_top`
* `torso_top`
* `torso_center`
* `torso_bottom`

This line is visually central because it expresses:

* body orientation
* posture
* compression or extension
* part of the CM-driven motion logic

### 8.3 Limb Branches

Each limb branch should be visually derived from its articulation chain:

* left arm: `torso_top -> elbow_left -> wrist_left`
* right arm: `torso_top -> elbow_right -> wrist_right`
* left leg: `torso_bottom -> knee_left -> ankle_left`
* right leg: `torso_bottom -> knee_right -> ankle_right`

### 8.4 Shared Branch Junctions

Special care will be needed at:

* `torso_top`
* `torso_bottom`

because those are branching points in the visible topology.

The rendering model must avoid making these junctions look broken or mechanically pin-jointed.

---

## 9. Head Representation

The head is a special case because the internal body model defines it through:

* `torso_top`
* `head_top`

and [01_Stickman_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/01_Stickman_Model.md) interprets that vertical span as the head diameter.

### 9.1 Nominal Head Geometry

The head should remain visually consistent with the geometric model:

* `head_diameter = L_rest`
* `head_radius = 0.5 * L_rest`

### 9.2 Rendering Interpretation

The simplest rendering interpretation is a circle or near-circular stroke attached to the upper body line.

This is visually compatible with the rest of the continuous-line character as long as the head attachment remains clean.

### 9.3 Head Attachment

The connection between head and trunk should not look like:

* a detached circle
* a rigid weld artifact
* a visible gap

The rendering layer should therefore treat the head/trunk connection as a visual continuity problem, not just as a geometric coincidence of two points.

---

## 10. Stroke Thickness and Screen Presence

The project should not think only in terms of centerlines.
The visible figure also needs a readable stroke thickness.

### 10.1 Why Thickness Matters

Thickness affects:

* silhouette readability
* visual stability during motion
* screen presence at small scales
* whether the stickman feels intentional or merely skeletal

### 10.2 Thickness Should Be a View Parameter

Stroke thickness belongs to rendering.
It should not be stored as a locomotion or simulation quantity.

This is another reason why rendering must remain separate from the body model.

### 10.3 Future Variations

Later, thickness may vary slightly according to:

* zoom level
* screen resolution
* visual style decisions

But those choices remain part of the View, not of the Model.

---

## 11. Coordinate Conversion and SDL

The rendering layer must transform model-space body data into screen-space drawing data.

### 11.1 Model Space

The internal model uses:

* world/simulation coordinates
* upward-positive vertical geometry
* physically readable positions

as defined in [01_Stickman_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/01_Stickman_Model.md).

### 11.2 Screen Space

SDL will render in:

* pixel coordinates
* a screen-space origin
* a vertical axis that typically grows downward

### 11.3 Why the Conversion Must Stay in the View

The conversion from model-space to screen-space must remain a rendering concern.
It should not contaminate:

* the locomotion model
* the CM model
* the support logic
* the articulated body state

If this boundary is not kept clean, the project risks mixing physics and rendering conventions in harmful ways.

---

## 12. Rendering and MVC

The rendering model must remain fully compatible with the required MVC architecture.

### 12.1 What Belongs to the Model

The Model should contain:

* body state
* node positions
* support state
* locomotion regime
* CM state

### 12.2 What Belongs to the View

The View should contain:

* model-space to screen-space conversion
* spline or curve generation for drawing
* stroke generation
* head drawing
* debug overlays if needed

### 12.3 What the Controller Should Not Do

The Controller should not directly build final rendered curves.
Its role is to orchestrate behavior, not to own visual geometry.

This separation is essential because the same locomotion state should remain renderable in different visual styles without changing the simulation logic.

---

## 13. Contact Visibility and Terrain Compatibility

The rendering model must also remain compatible with the support and terrain ambitions of the project.

### 13.1 Contact Must Stay Legible

The viewer should be able to read:

* which leg is supporting
* when support changes
* when the body is in flight
* when a recovery step lands
* when a fall contacts the ground

The rendered line should therefore not obscure or confuse support events.

### 13.2 Future Terrain Complexity

Even if V1 begins on flat ground, the rendering model should remain visually valid later for:

* slopes
* uneven terrain
* rough surfaces
* discontinuous footholds

This means the rendering should not assume that:

* feet always align to a horizontal ground line
* the world is always symmetric around flat support
* branch angles remain visually simple

The rendering system should be able to display the same character coherently over more complex support geometry.

---

## 14. Debug Rendering vs Final Rendering

The project will likely need at least two rendering modes.

### 14.1 Debug Rendering

Debug rendering may show:

* raw node positions
* segment links
* CM location
* support points
* terrain contact hints

This is useful for development and validation.

### 14.2 Final Character Rendering

Final rendering should emphasize:

* continuous body appearance
* visual clarity
* animated character identity

The debug skeleton should not be confused with the final visual body.

This distinction is important because both are useful, but for different purposes.

---

## 15. Main Rendering Failure Modes to Avoid

The following rendering design errors should be avoided:

* drawing the internal skeleton directly as the final character
* coupling simulation logic to SDL coordinate conventions
* storing rendering-only quantities inside the locomotion model
* making branch junctions look visibly disconnected
* making perturbation transitions look like visual body swaps
* hiding support events so much that locomotion becomes hard to read
* hard-coding the visual model to flat-ground assumptions

These are not minor aesthetic issues.
They would weaken the whole project.

---

## 16. What Is Deferred to Later Documents

This document does not yet define:

* exact spline family selection
* exact control-point generation formulas
* exact stroke tessellation
* exact anti-aliasing strategy
* exact SDL rendering API usage
* exact caching or performance strategy
* exact rendering module interfaces

Those belong to later software and implementation documents.

---

## 17. Summary of Main Takeaways

1. The internal stickman and the rendered character are not the same thing.
2. The internal model is control-oriented; the rendering model is appearance-oriented.
3. The final character should read as one continuous line-based body, not as a raw articulated skeleton.
4. Smooth curves or splines are needed to transform the simplified node graph into a coherent visible figure.
5. Rendering control points may be richer than the physical node set.
6. Model-space to screen-space conversion and curve generation belong to the View, not to the Model or Controller.
7. The rendering model must remain readable under perturbation, recovery, falling, and future terrain complexity.
