# Design Docs

This directory contains the **conceptual design dossier** of the project.

These files are still useful and important, but they are not all at the same level of authority.

## Reading Rule

Use the files in this directory to understand:

- project intent
- control logic
- rendering intent
- physics intent
- recovery intent
- validation philosophy

Do **not** treat this directory alone as the final coding skeleton.

For actual implementation decisions, always cross-check:

- [`../implementation/README.md`](../implementation/README.md)
- [`../implementation/05_Midpoint_Procedural_Locomotion_Spec.md`](../implementation/05_Midpoint_Procedural_Locomotion_Spec.md)
- [`../implementation/06_Design_To_Implementation_Mapping.md`](../implementation/06_Design_To_Implementation_Mapping.md)

## Practical Precedence

- `design/` explains the conceptual target
- `implementation/` explains what the code should look like now

If a conceptual design detail conflicts with an implementation-facing file on:

- class ownership
- source tree layout
- midpoint scope
- runtime composition
- debug ownership

then the file in `implementation/` takes precedence.

## Midpoint Reminder

For midpoint:

- the active path is procedural-only
- `Stand`, `Walk`, and `Run` are the only required active behaviors
- `GaitPhase` is a dedicated gait-support type
- the physical branch is reserved but inactive
