# Midpoint Implementation Progress

This directory records the implementation progress of the midpoint deliverable.

The goal of this log is not to replace the architecture documents.
Its role is to explain, step by step:

- what was implemented
- why it was implemented that way
- what remains intentionally deferred
- which responsibilities are delegated to other teammates

## Ownership Split For The Current Track

The current track owned in this branch covers:

- stickman geometry
- solved stickman node positions
- raw skeleton rendering
- `Stand`
- `Walk`
- `Run`

The following items are intentionally delegated to teammates:

- console debug progression
- spline / continuous-line rendering progression

## Log Entries

### 2026-03-12 - Runtime Skeleton Ready

The runtime and core architecture skeleton is now in place.

At this point the project can:

- open and close the SDL window correctly
- switch high-level locomotion modes through the new runtime structure
- build both native and web targets

The next implementation phase focuses on making the mannequin visible with a raw skeleton before
any continuous-line rendering work starts.

### 2026-03-12 - Geometry Types And Naming Alignment

The codebase now introduces explicit mannequin geometry and node-position value types.

This step is intentionally done before any visible skeleton rendering because the implementation
must match the body model vocabulary already frozen in the design dossier.

The important decision here is naming alignment:

- code now uses `HeadTop` instead of a generic `Head`
- mannequin node positions are represented explicitly in world space
- the nominal stickman geometry is now a first-class data object

This reduces future ambiguity when reconstruction and rendering start consuming the same body
graph.

Spline and continuous-line rendering remain delegated to teammates.

### 2026-03-12 - Character State Publishes Visible Body Data

The authoritative `CharacterState` now carries:

- the nominal stickman geometry
- the solved world-space node positions of the body

This is an important boundary decision.

The simulation core is now responsible for publishing the visible mannequin snapshot, even though
the renderer still does not draw it yet.

This keeps the future raw skeleton renderer and the future continuous-line renderer dependent on
the same authoritative body data instead of rebuilding geometry independently.

### 2026-03-12 - First Visible Raw Skeleton

The project now renders a first visible mannequin as a raw skeleton:

- straight limb segments
- visible joint markers
- a simple head circle

This is intentionally not the final visual style.

The goal of this step is to make the body graph visible and debuggable before the continuous-line
rendering work starts in the teammate track.

At this stage the standing pose is still a first explicit readable layout.
The next step will make that pose more rigorously geometry-driven from the stickman model itself.

### 2026-03-12 - Standing Pose Driven By Body Geometry

The standing mannequin is no longer positioned from arbitrary segment multiples.

It is now reconstructed from:

- nominal leg lengths
- nominal torso segment lengths
- nominal head radius
- a fixed standing support width
- a neutral arm hanging direction

This is still a deliberately simple midpoint standing solver.
The important change is architectural:

- the visible pose is now derived from body geometry
- the pelvis and the CM reference are expressed in body-scale units
- the standing posture becomes the baseline reference for later `Walk` and `Run`

The renderer is still the same raw skeleton renderer.
Spline rendering and console-debug progression remain delegated to teammates.

### 2026-03-12 - Standing Pose Gets A Nominal Facing Direction

The standing mannequin now carries a nominal facing direction, currently initialized to face right.

This does not introduce locomotion yet.
The goal is visual readability:

- the torso remains vertically aligned in standing
- the feet stay symmetric around the CM projection
- the facing side is suggested through asymmetric bending of the limbs instead of a tilted spine

The result is still a raw skeleton, but it should read less like a flat technical diagram and
more like a character with a clear orientation in space.

### 2026-03-12 - Walk Timing Now Matches The Midpoint Spec

The locomotion controller now treats `Stand` and `Walk` as two different timing regimes.

This step is intentionally state-level only:

- `Stand` now uses `GaitPhase::None`
- `Walk` now follows the frozen midpoint phase partition
- the renderer is not animating those phases yet

This is the correct next step because the animator should consume a valid gait-state machine
instead of inventing phase timing on its own.

`Run` timing is still the next pending step after this one.

### 2026-03-12 - Ground Reference Added To The Raw Renderer

The raw renderer now draws a simple visual ground reference:

- one horizontal ground line
- a few small tick marks along the floor

This is still not a physics ground.
It is a visual debugging aid added before walk animation so that foot locking, foot lift, and
possible foot sliding can be judged against a visible baseline.
