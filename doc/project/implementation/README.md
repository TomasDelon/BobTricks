# Implementation Freeze

This directory contains the implementation-facing architecture freeze for the project.

These documents are the source of truth for coding decisions.
If an older conceptual design document conflicts with these files on concrete software structure,
the files in this directory take precedence.

## Reading order

1. `00_Implementation_Freeze.md`
2. `05_Midpoint_Procedural_Locomotion_Spec.md`
3. `01_Runtime_Architecture.md`
4. `02_State_Model.md`
5. `03_Debug_Architecture.md`
6. `04_Source_Tree_And_Ownership.md`
7. `06_Design_To_Implementation_Mapping.md`

## Scope

The architecture is defined for:

- the midpoint demo in one week
- the final course deliverable

The midpoint demo only activates a subset of the final architecture, but the codebase must start
with the final structure in mind.
