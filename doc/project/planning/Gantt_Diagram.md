# Gantt Diagram

## 1. Role of This Document

This document provides the planning structure of the project in a form compatible with the future course deliverable.

It is based on the task breakdown defined in [Cahier_des_Charges.md](/home/tomas/UL1/LIFAPCD/phy/doc/cahier_des_charges/Cahier_des_Charges.md).

The planning must now respect the following course milestones:

* `2026-03-09`: cahier des charges
* `2026-03-17`: mid-project demo
* `2026-04-28`: final soutenance

The internal objective of the group is to have the project functionally ready by:

* `2026-04-25`

At this stage, what matters most is:

* task ordering
* major dependencies
* workload grouping

---

## 2. V1 Planning with Course Milestones

```mermaid
gantt
title V1 Project Planning
dateFormat  YYYY-MM-DD
axisFormat  %d/%m

section Design
Finalize scope and CDC                 :crit, d1, 2026-03-03, 2026-03-09
Refine class diagram                  :d2, 2026-03-05, 5d
Define code structure and interfaces  :d3, 2026-03-07, 4d
CDC submission milestone              :milestone, m1, 2026-03-09, 0d

section Core Implementation
Set up C++/SDL/Box2D/ImGui/Eigen      :i1, 2026-03-06, 4d
Implement stickman state and geometry :i2, 2026-03-08, 4d
Implement nominal locomotion modes    :i3, 2026-03-10, 10d
Implement CM controller               :i4, 2026-03-10, 7d
Implement reconstruction and IK       :i5, 2026-03-14, 8d
Implement gait phase logic            :i6, 2026-03-18, 4d

section Continuous Validation
Validate geometry and state updates   :cv1, 2026-03-11, 3d
Validate nominal locomotion locally   :cv2, 2026-03-18, 4d
Validate reconstruction and IK locally:cv3, 2026-03-22, 4d

section Physical and Hybrid
Implement Box2D physical body         :p1, 2026-03-14, 7d
Implement support/contact reasoning   :p2, 2026-03-20, 5d
Implement recovery metric             :p3, 2026-03-24, 4d
Implement procedural-physical switch  :p4, 2026-03-25, 5d
Implement falling and stabilization   :p5, 2026-03-30, 5d
Implement bounded get-up              :p6, 2026-04-04, 5d

section Continuous Debug
Add core debug overlays               :cd1, 2026-03-12, 4d
Add state inspection panels           :cd2, 2026-03-28, 4d

section Rendering and Debug
Implement continuous-line rendering   :r1, 2026-03-16, 6d
Implement render-state adapter        :r2, 2026-03-22, 4d
Implement debug overlays and ImGui    :r3, 2026-03-26, 5d

section Integration Validation
Prepare repeatable test scenarios     :v1, 2026-03-28, 4d
Validate nominal behaviors            :v2, 2026-04-01, 5d
Validate perturbation and recovery    :v3, 2026-04-10, 6d
Tune transitions and contact          :v4, 2026-04-16, 6d

section Delivery
Prepare mid-project demo              :crit, f1, 2026-03-13, 2026-03-17
Mid-project demo milestone            :milestone, m2, 2026-03-17, 0d
Write README and Doxygen              :f2, 2026-04-10, 5d
Prepare final presentation            :f3, 2026-04-20, 4d
Clean archive and final checks        :crit, f4, 2026-04-23, 2026-04-25
Project-ready internal milestone      :milestone, m3, 2026-04-25, 0d
Final soutenance milestone            :milestone, m4, 2026-04-28, 0d
```

---

## 3. Practical Reading

The plan is intentionally organized in overlapping streams:

* design
* core implementation
* physical/hybrid behavior
* rendering/debug
* validation
* delivery

This reflects the fact that the project is not purely sequential.
Some technical parts can advance in parallel once the design foundation is stable enough.
It also reflects an important project rule: validation and debugging should happen continuously during development, not only at the end.
It also reflects the academic constraint that the project must be document-ready on `2026-03-09`, demo-ready on `2026-03-17`, and functionally ready before `2026-04-25`.

---

## 4. Approximate Task Ownership

The following responsibility split is approximate and may evolve during implementation, but it already provides a readable basis for the course deliverable:

* **Tomas Delon**: overall integration, conception documents, class diagram, CM-related logic, reconstruction/IK, and final technical consistency
* **Maleik**: physical body, Box2D integration, support/contact handling, recovery logic, and hybrid transitions
* **Gedik**: SDL rendering, continuous-line visual representation, Dear ImGui debug tools, and presentation-oriented visual polish

Some tasks are naturally shared:

* validation and debugging
* integration of nominal and physical behaviors
* final documentation and delivery preparation

---

## 5. Expected Mid-Project Demo

The `mi-parcours` presentation on `2026-03-17` is expected to show a coherent partial prototype rather than the full V1 feature set.

The realistic target for that milestone is:

* a project that builds and runs correctly
* an SDL window with the initial stickman representation
* the first version of the state and geometry pipeline
* at least a partial nominal behavior, such as standing and an initial locomotion behavior
* early reconstruction/IK integration
* initial debug overlays or inspection tools useful for demonstration

This milestone is intended to prove that the technical base is functional and that the main architectural choices are already implementable.

---

## 6. Future Update

Before final submission, this planning should be updated into a final Gantt diagram that:

* reflects the real work done
* identifies who contributed to which tasks
* is exportable in an image or PDF format suitable for the course deliverable
