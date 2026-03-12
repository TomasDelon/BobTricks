# Software Architecture MVC

## 1. Role of This Document

This document defines the **software-level MVC architecture** of the project.

The previous documents already established:

* what the project wants to build
* what the stickman body model is
* how the CM is interpreted
* how the hybrid regimes work
* how the control architecture is layered
* how the final rendering should look

What is still missing is the explicit software decomposition required by the project constraints.

This document therefore answers the following questions:

* What belongs to the **Model**?
* What belongs to the **View**?
* What belongs to the **Controller**?
* How should data move between them?
* Where should Box2D fit?
* Where should SDL fit?
* Which mistakes would break the architecture later?

This document does **not** yet define:

* exact class names
* exact APIs
* exact folder structure
* exact update-loop implementation
* exact dependency injection strategy

Those belong to later software design documents.

---

## 2. Why MVC Must Be Taken Seriously

MVC is not a cosmetic requirement in this project.
It is structurally necessary.

The project combines:

* locomotion logic
* hybrid regime switching
* physical simulation
* contact and support reasoning
* spline-based visual rendering
* debug rendering
* future terrain complexity

If these concerns are mixed too early, the codebase will quickly become difficult to:

* reason about
* extend
* debug
* validate

This is especially important here because the project deliberately separates:

* the **internal articulated model**
* the **rendered visible body**

So the architecture must preserve that separation in software as well.

---

## 3. MVC in the Context of This Project

In this project, MVC should be understood in the following way.

### 3.1 Model

The Model is the source of truth for:

* body state
* CM state
* locomotion regime state
* support and contact state
* physical state
* terrain state
* procedural targets

The Model contains the simulation-side reality of the character.

### 3.2 View

The View is responsible for:

* converting simulation data into visible graphics
* building curves, splines, strokes, and debug overlays
* converting model-space coordinates into SDL screen-space coordinates
* presenting the final line-based stickman

The View should not decide locomotion, support, or physics.

### 3.3 Controller

The Controller is responsible for:

* receiving external commands or user intentions
* triggering perturbations
* selecting or requesting locomotion actions
* orchestrating the interaction between user intent and system logic

The Controller should not become the owner of body truth or rendered geometry.

---

## 4. What Belongs to the Model

The Model is the most important part to protect.
If it gets polluted with SDL or presentation logic, the project will become harder to evolve.

### 4.1 Body Geometry and Kinematic State

The Model should contain:

* stickman node positions
* articulated hierarchy state
* nominal geometric parameters
* current reconstructed body configuration

This comes directly from [01_Stickman_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/01_Stickman_Model.md).

### 4.2 CM and Locomotion State

The Model should contain:

* CM position
* CM velocity
* CM acceleration when needed
* current CM operating height
* current locomotion mode
* current gait phase when relevant

This comes directly from [02_CM_And_Locomotion_Models.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/02_CM_And_Locomotion_Models.md).

### 4.3 Hybrid Regime State

The Model should contain:

* current hybrid regime
* recoverability status
* groundedness state
* fall state
* get-up readiness

This comes directly from [03_Hybrid_Control_Regimes.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/03_Hybrid_Control_Regimes.md).

### 4.4 Support, Contact, and Terrain State

The Model should contain:

* active support information
* contact information
* support validity
* capturability-related information
* terrain description needed by the locomotion logic

This is critical for future support of:

* slopes
* uneven terrain
* rough surfaces
* discontinuous footholds

### 4.5 Procedural Targets and Control-Side State

The Model may also contain:

* current high-level locomotion intent
* current procedural pose targets
* current foot placement targets
* current reconstruction targets

These are still simulation/control data, not rendering data.

### 4.6 Physics Engine State

If the project uses Box2D, the physical simulation state belongs conceptually to the Model.

This includes:

* rigid-body state
* joint state
* contact results
* physical response state

The important point is that Box2D is part of the **simulation-side Model**, not of the View and not of the Controller.

---

## 5. What Belongs to the View

The View exists to show the state of the Model.
It should not redefine that state.

### 5.1 Screen Conversion

The View should perform:

* model-space to screen-space conversion
* camera or viewport transforms if needed
* scaling to pixel coordinates

This follows directly from [07_Rendering_Model.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/07_Rendering_Model.md).

### 5.2 Continuous-Line Character Rendering

The View should contain:

* curve generation
* spline generation
* stroke generation
* head rendering
* final visible body drawing

These quantities are visual, not physical.

### 5.3 Debug Rendering

The View may also render:

* raw skeleton nodes
* segment links
* CM marker
* support markers
* terrain contact hints
* regime labels

This is still View logic, because it is presentation of existing state.

### 5.4 Rendering Parameters

The View should own rendering-only quantities such as:

* stroke thickness
* colors
* screen-space smoothing parameters
* debug visibility flags

These should not be stored as locomotion or body truths.

### 5.5 SDL

SDL belongs primarily to the View layer.

This means:

* window creation
* render target management
* drawing operations
* presentation to screen

should all remain View-side concerns.

### 5.6 Dear ImGui

If the project uses **Dear ImGui** for debugging and development tooling, it should be treated as part of the View-side and Controller-side tooling boundary rather than as part of the simulation Model.

Typical uses include:

* debug sliders
* state inspection panels
* toggles for overlays and test scenarios

ImGui should help inspect the simulation, not become the place where locomotion truth is stored.

---

## 6. What Belongs to the Controller

The Controller is the orchestration layer between outside requests and the simulation state.

### 6.1 Input Interpretation

The Controller should receive and interpret:

* user actions
* debug commands
* scenario events
* scripted perturbations

### 6.2 Action Requests

The Controller should translate those into requests such as:

* start walking
* run
* crouch
* jump
* inject perturbation
* trigger get-up
* switch debug rendering mode

### 6.3 High-Level Orchestration

The Controller may coordinate:

* when an intent request is sent to the Model
* when a perturbation event is introduced
* when simulation stepping is paused or resumed
* when debug output is toggled

### 6.4 What the Controller Must Not Own

The Controller should not own:

* the authoritative body state
* the authoritative CM state
* Box2D simulation internals
* final spline geometry

Otherwise it would stop being an orchestrator and become a confused mixture of logic layers.

---

## 7. Data Flow Between MVC Layers

The architecture should remain readable as a directional flow of responsibility.

### 7.1 Typical Flow

A typical update-render cycle can be understood conceptually as:

1. the Controller receives input or scripted events
2. the Controller expresses high-level requests to the Model
3. the Model updates simulation, control, physics, and body state
4. the View reads the resulting Model state
5. the View renders the visible character and debug overlays

### 7.2 Important Consequence

The View should **read** state, not invent simulation truth.

The Controller should **request** state changes, not directly redraw the body truth.

The Model should **evolve** the simulation state, not build final screen geometry.

---

## 8. Where the Internal Control Layers Fit

The control architecture defined in [04_Control_Architecture.md](/home/tomas/UL1/LIFAPCD/phy/doc/design/04_Control_Architecture.md) fits primarily inside the Model side of MVC.

### 8.1 Model-Side Control Logic

The following layers belong conceptually to the Model domain:

* Intent Layer state
* Regime Manager
* CM Controller
* Support and Contact Reasoning
* Foot Placement Planner
* Articulated Reconstruction
* Physical Tracking logic
* Failure Handling

This may feel surprising at first, because some of these sound like "controllers" in the ordinary English sense.

But under MVC they are not UI Controllers.
They are part of the **simulation/control model** of the character.

### 8.2 Why This Distinction Matters

If these layers were placed inside the MVC Controller just because they control behavior, the Controller would become:

* too large
* too stateful
* too coupled to simulation internals

That would damage the software structure.

---

## 9. Box2D and MVC

This project should not reinvent a physics engine.
If Box2D is used, its place in MVC must be clear.

### 9.1 Box2D Belongs to the Simulation Side

Box2D should be considered part of the Model-side simulation machinery.

It provides:

* rigid-body dynamics
* joints
* contacts
* collisions
* integration

### 9.2 What Uses Box2D

Model-side systems may interact with Box2D to:

* read physical body state
* apply control torques, impulses, or motor targets
* inspect contacts
* detect groundedness and collision events

### 9.3 What Box2D Does Not Replace

Box2D does not replace:

* locomotion intent
* hybrid regime logic
* CM-level locomotion modeling
* support reasoning at the project level
* rendering

So Box2D is a physics subsystem inside the Model, not the architecture itself.

---

## 10. SDL and MVC

SDL should be treated primarily as the rendering and application I/O layer, not as the owner of character logic.

### 10.1 SDL as View Infrastructure

SDL should support:

* windows
* frame presentation
* drawing surfaces
* event retrieval

### 10.2 SDL Events and the Controller

SDL input events may be received by the Controller, but they should not directly mutate rendering-independent simulation structures in ad hoc ways.

Instead, they should be interpreted into:

* intent changes
* perturbation requests
* debug toggles
* application commands

### 10.3 Why This Matters

Without this discipline, SDL event handling can easily leak UI concerns deep into the simulation model.

---

## 11. Recommended Architectural Boundaries

The most useful practical boundaries are the following.

### 11.1 Simulation Boundary

Everything needed to answer:

* what the body is doing
* what the CM is doing
* what support is valid
* whether recovery is possible

belongs on the Model side.

### 11.2 Rendering Boundary

Everything needed to answer:

* how the body is drawn
* how thick the stroke is
* which splines are used
* how the camera maps model-space to screen-space

belongs on the View side.

### 11.3 Orchestration Boundary

Everything needed to answer:

* what the user requested
* what debug mode is active
* which high-level command is sent to the simulation

belongs on the Controller side.

---

## 12. Main Architecture Failure Modes to Avoid

The following mistakes should be explicitly avoided.

### 12.1 Putting SDL Coordinates Into the Model

The Model should not store body state in screen coordinates.

### 12.2 Putting Curve Geometry Into the Physics State

The Model should not treat spline control points as if they were physical joints.

### 12.3 Letting the Controller Own the Simulation Truth

The Controller should not become the place where:

* body state is stored
* hybrid regime is decided ad hoc
* Box2D state is manually mirrored without discipline

### 12.4 Letting the View Decide Physics

The View should never decide:

* whether a fall happened
* whether support is valid
* whether recovery is possible

### 12.5 Flat-Ground Assumptions Hidden in the Wrong Layer

Future terrain complexity will be much harder if flat-ground assumptions are buried inside:

* rendering code
* event-handling code
* miscellaneous helper utilities

Terrain interpretation should stay on the Model side.

---

## 13. Why This MVC Structure Helps Future Work

This architecture is not only about cleanliness.
It directly supports later project growth.

It makes it easier to:

* change the visual style without rewriting locomotion logic
* improve physical simulation without rewriting SDL drawing
* add more debug views without changing the control model
* extend from flat ground to more complex terrain
* build validation tools that inspect model truth independently of rendering style

This is one of the strongest reasons to keep the architecture clean from the start.

---

## 14. What Is Deferred to Later Documents

This document does not yet define:

* exact module names
* exact class diagram
* exact ownership semantics
* exact update-loop ordering
* exact serialization strategy
* exact memory layout

These belong to later software-structure documents.

---

## 15. Summary of Main Takeaways

1. The Model contains the simulation-side truth of the character, including locomotion, support, hybrid regime state, and Box2D state.
2. The View contains SDL rendering, curve generation, stroke presentation, and debug visualization.
3. The Controller interprets input and orchestrates high-level requests, but should not own simulation truth or rendered geometry.
4. The internal locomotion and control layers belong conceptually to the Model side of MVC.
5. Box2D belongs inside the simulation side of the architecture, while SDL belongs primarily to the View side.
6. Clean MVC boundaries are essential for future terrain complexity, validation, and rendering evolution.
