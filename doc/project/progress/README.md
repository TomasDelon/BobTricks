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
