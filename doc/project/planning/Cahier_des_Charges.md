# Cahier des Charges

## 1. Presentation Of The Project

### 1.1 Title

**CM-Driven Procedural Stickman Locomotion**

### 1.2 Team

Current team:

- Tomas Delon
- Maleik
- Gedik

To be completed before submission with:

- full surnames where still missing
- student numbers
- forge project identifier

### 1.3 Short Project Summary

This project is developed in the context of the **LIFAPCD** course at Universite Claude Bernard
Lyon 1.

Its goal is to build a **2D procedural stickman application** whose motion is generated from
procedural control rather than from prerecorded animation clips.

The project is intentionally staged:

- the midpoint demo focuses on a procedural-only locomotion core
- the final deliverable extends the same architecture toward richer behaviors and reactive logic

### 1.4 Motivation

This project combines several themes of the course in one coherent application:

- software design and modular architecture
- UML and planning
- real-time programming
- graphics and GUI with SDL
- procedural animation
- possible later integration of physical simulation

It is also technically interesting because the project does not simply play fixed animations.
Instead, it attempts to generate motion from simplified biomechanical principles, support logic,
and articulated reconstruction.

---

## 2. Objectives And Scope

### 2.1 Main Objective

The main objective is to create a **2D procedural stickman locomotion system** that can:

- remain readable as a continuous-line character
- produce intentional nominal locomotion without prerecorded clips
- remain architecturally extensible toward richer future behaviors

### 2.2 Midpoint Objective

The midpoint demo objective is intentionally narrow.

The midpoint must demonstrate:

- `Stand`
- `Walk`
- `Run`

The midpoint runtime is **procedural only**.
It must not depend on Box2D to be considered successful.

### 2.3 Final Objective

The final target extends the same architecture toward:

- `Stand`
- `Walk`
- `Run`
- `Crouch`
- `Jump`
- `Land`
- `Fall`
- `Recovery`
- `GetUp`

The final architecture may activate a physical authority branch if it becomes necessary, but that
branch is not part of the midpoint critical path.

### 2.4 Midpoint Scope

The midpoint deliverable includes:

- a 2D stickman with procedural `Stand`, `Walk`, and `Run`
- timer-driven gait progression
- analytic or lightweight procedural reconstruction for the simplified body
- continuous-line rendering in SDL
- a minimal but real debug backend
- console and ImGui debug frontends sharing the same backend
- project documentation coherent with the course expectations

### 2.5 Final-Scope Extensions

The following are valid final-scope extensions after the midpoint:

- crouching
- jumping and landing
- falling and grounded stabilization
- recovery logic
- get-up logic
- optional physical authority with Box2D

### 2.6 Out Of Scope For The Midpoint

The following are explicitly outside the midpoint scope:

- full physical authority in nominal locomotion
- recovery classification and recovery stepping
- falling and grounded stabilization
- generalized get-up behavior
- complete terrain adaptation
- whole-body optimization or HQP
- full anthropometric realism

---

## 3. Detailed Description Of The Application

### 3.1 General Principle

From the user point of view, the application presents a 2D stickman capable of moving in real
time.

The character should appear as a single continuous visible figure rather than as disconnected debug
segments.

At midpoint, the focus is on nominal procedural locomotion.
In the final version, the same project may evolve toward richer reactive and recovery behavior.

### 3.2 Core Interaction Model

The application is primarily an interactive simulation and demonstration project rather than a
traditional game.

At midpoint, the user should be able to:

- trigger `Stand`, `Walk`, and `Run`
- inspect internal state through debug tools
- observe the character response to different locomotion commands

The application should provide visual feedback about:

- the current locomotion mode
- the current gait phase
- the visible posture and support organization
- optional debug overlays useful during development and demonstration

### 3.3 Expected Visual Result

The internal body model is simplified and articulated, but the final visible result should not look
like a raw debug skeleton.

The character should be rendered in SDL as a **continuous line-like figure**, giving the project a
clean and stylized visual identity.

Dear ImGui may be used for development-time debug panels and tuning support, but it must remain a
replaceable frontend over a backend-owned debug system.

---

## 4. Technical And Design Choices

### 4.1 Midpoint Technology Stack

The midpoint stack is:

- **C++**
- **SDL2**
- **CMake**
- **Dear ImGui** for debug UI

Midpoint should rely on lightweight project-owned math types when possible.

### 4.2 Planned Final Extensions

The architecture is prepared to accept later additions such as:

- **Box2D** for physical authority and reactive behavior
- **Eigen**, only if a clear mathematical need justifies it

These are not midpoint requirements.

### 4.3 Software Architecture

The software architecture follows the constraints of the course and the design dossier:

- **MVC** is mandatory
- the Model contains simulation logic and authoritative state
- the View contains SDL rendering and visual debug presentation
- the Controller layer interprets inputs and debug commands through clean abstractions

Additional architectural rules:

- the core must run without SDL and without ImGui
- the debug backend must be independent from console and ImGui
- the root runtime object is `Application`
- the renderer must not own event polling or the main loop

### 4.4 Design Constraints

The following constraints are already fixed:

- the project is strictly **2D**
- midpoint nominal motion is **procedural / kinematic**
- the visible stickman must be rendered as a **continuous line-like figure**
- the design must remain extensible without pretending to solve every advanced feature at midpoint

---

## 5. Functional Requirements

### 5.1 Mandatory Midpoint Requirements

The midpoint implementation must:

- provide a working executable application
- implement procedural `Stand`, `Walk`, and `Run`
- keep walking and running visually distinguishable
- use timer-driven gait progression
- provide usable SDL rendering of the continuous-line stickman
- provide a minimal but useful debug system for development and demonstration

### 5.2 Midpoint Behavioral Requirements

The midpoint character must satisfy the following:

- standing remains visually stable under no disturbance
- walking alternates support coherently and does not introduce flight
- running contains a real flight phase
- running is not only a faster copy of walking
- stance feet do not slide during nominal support

### 5.3 Midpoint Interface And Visualization Requirements

The midpoint application must satisfy the following:

- the main visible character remains readable as a continuous-line body
- debug information remains clearly secondary to the main character
- console and ImGui expose the same debug capability set conceptually
- debug tools must support state inspection, time scaling, and trajectory tracing

### 5.4 Build And Execution Requirements

To satisfy course conventions, the project must:

- be buildable with `CMake`
- keep source code, documentation, binaries, and assets clearly organized
- document compilation and execution steps in the root `README.md`
- remain compatible with the university Linux environment

### 5.5 Final Deliverable Extensions

Later phases may add:

- crouching
- jumping and landing
- recovery and falling
- grounded stabilization
- get-up
- physical authority through Box2D

---

## 6. Non-Functional Requirements

### 6.1 Code Organization

The codebase must remain:

- modular
- readable
- sufficiently documented
- consistent with the architecture freeze

The project should avoid monolithic classes and should respect the module decomposition prepared in
the design and implementation documents.

### 6.2 Reliability

The application is not expected to be industrial software, but it must remain reliable enough for
course deliverables:

- no major crashes in normal use
- stable nominal execution
- coherent mode transitions
- no obvious visual or timing explosions during the midpoint demo

### 6.3 Portability And Environment

The baseline environment is the university Linux context:

- Linux is the primary target platform
- SDL2 must be available or installable in a course-compatible way
- any dependency beyond the midpoint stack must remain justified

### 6.4 Real-Time Performance

The application must run at real-time speed during normal midpoint demo scenarios.

In particular:

- the fixed simulation step must remain executable at interactive speed
- the visible application must remain fluid enough for classroom demonstration
- debug mode may add overhead, but it must not make the application unusable

---

## 7. Task Breakdown

### 7.1 Midpoint Design Tasks

The design phase must include:

- freeze the midpoint scope
- freeze the implementation architecture
- freeze the midpoint procedural locomotion specification
- maintain a coherent class and module dossier
- maintain a coherent workflow for code publication

### 7.2 Midpoint Implementation Tasks

The midpoint implementation phase must include:

- create the base project structure in C++
- configure SDL2 and ImGui
- implement the abstract stickman state and geometric parameters
- implement gait phase logic
- implement procedural `Stand`
- implement procedural `Walk`
- implement procedural `Run`
- implement the procedural CM and pose update pipeline
- implement render-state adaptation
- implement continuous-line rendering
- implement the shared debug backend
- implement console and minimal ImGui debug frontends

### 7.3 Final Extension Tasks

The later implementation phase may include:

- configure Box2D if needed
- implement crouch, jump, land, fall, recovery, and get-up
- implement physical authority and readback
- implement recovery metrics and reactive stepping
- implement richer transitions between procedural and physical authority
