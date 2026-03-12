# V1 TODO

## 1. Before 2026-03-09

* finalize surnames, student numbers, and forge identifier in [Cahier_des_Charges.md](/home/tomas/UL1/LIFAPCD/phy/doc/cahier_des_charges/Cahier_des_Charges.md)
* export the current class diagram draft into a course-friendly image or PDF
* export the current Gantt diagram into a course-friendly image or PDF
* do one final spelling and formatting pass on the `cahier des charges`

## 2. Before 2026-03-17

* create the base C++ project structure
* integrate SDL, Box2D, Dear ImGui, and Eigen
* implement the first stickman geometry and state containers
* render the first visible stickman in an SDL window
* implement the first nominal state update path
* implement the first debug overlays
* prepare a realistic `mi-parcours` demo scenario

## 3. Core Nominal Locomotion

* implement standing behavior
* implement crouching behavior
* implement walking behavior
* implement running behavior
* implement nominal jumping and landing
* implement the first discrete gait-phase logic
* implement the nominal `CM` controller
* validate each nominal behavior before moving to the next one

## 4. Reconstruction and Pose

* implement the procedural pose state
* implement analytic 2-link leg IK
* implement analytic arm reconstruction
* implement the trunk target update from `CM` targets
* implement regime-dependent reconstruction outputs
* validate reconstruction locally with debug views

## 5. Physical and Hybrid Behavior

* implement the Box2D physical body blueprint
* implement the Box2D world adapter
* implement contact collection
* implement support and contact reasoning
* implement the recovery metric
* implement the transition synchronizer
* implement the procedural-to-physical switch
* implement falling behavior
* implement grounded stabilization
* implement bounded get-up behavior
* implement physical-to-procedural resynchronization

## 6. Rendering and Debug

* implement the render-state adapter
* implement the continuous-line builder
* connect the curve output to SDL rendering
* implement readable debug overlays
* add Dear ImGui state inspection panels
* verify that rendering stays coherent with regime and support state

## 7. Continuous Validation

* validate geometry and segment lengths
* validate gait phase transitions
* validate nominal walking and running visually
* validate jump flight and landing coherence
* validate recovery classification on repeatable perturbation scenarios
* validate falling and grounded stabilization
* validate get-up from bounded grounded poses
* validate procedural/physical transitions

## 8. Final Integration

* tune nominal locomotion parameters
* tune support/contact behavior
* tune recovery and falling thresholds
* clean the code structure
* update UML and Gantt with the real implementation state
* write or complete the root `readme.md`
* generate Doxygen documentation
* prepare the final presentation
* prepare the final archive before `2026-04-25`

## 9. Approximate Ownership

* **Tomas Delon**: overall integration, design consistency, class diagram, `CM` logic, reconstruction, and final technical coherence
* **Maleik**: Box2D body, support/contact, recovery logic, and hybrid transitions
* **Gedik**: rendering, continuous-line view, Dear ImGui tooling, and presentation polish

Shared:

* validation and debug
* integration work
* documentation and final packaging
