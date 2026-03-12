# Stickman Model

## 1. Role of This Document

This document defines the **internal body model** of the stickman used by the project.
Its purpose is to specify:

* the node set
* the body topology
* the hierarchical structure
* the main geometric parameters
* the nominal standing references of the model

This document is intentionally **pedagogical**.
Whenever a result can be derived directly from the model definition, it is derived explicitly.
Whenever an assumption comes from the literature, the relevant source is indicated.
Whenever a simplification is a project choice rather than a scientific fact, it is stated as such.

This document does **not** yet define:

* locomotion control laws
* hybrid procedural/physical switching
* physical torque control
* terrain adaptation logic
* spline rendering rules
* software architecture or class design

Those topics belong to later documents.

---

## 2. Modeling Philosophy

The stickman is **not** intended to be a full anatomical human model.
It is a **simplified articulated figure** designed for:

* clarity
* controllability
* compatibility with CM-first locomotion
* later rendering as a continuous visual line

The model is therefore **topological and functional** rather than anatomically complete.

This choice is deliberate.
The project does not need every anatomical landmark of the human body.
It needs a body structure that is:

* simple enough to reason about
* rich enough to support locomotion, perturbation, and recovery
* compatible with a later smooth line-based rendering

Two kinds of statements will appear throughout this document:

* **literature-backed assumptions**
* **project-specific simplifications**

Keeping that distinction explicit is important, because this stickman is inspired by biomechanics, but it is not a medical or anthropometric reconstruction of the full human body.

---

## 3. External References and Modeling Assumptions

### 3.1 Literature-Backed Assumptions

The following assumptions are directly supported by the literature used in the project.

#### 3.1.1 The Body Can Be Approximated as a Segmented System

The body may be treated as a set of rigid segments when building simplified locomotion models.
This type of assumption appears explicitly in [Trajectory of the human body mass centre during walking at different speeds.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Trajectory%20of%20the%20human%20body%20mass%20centre%20during%20walking%20at%20different%20speeds.pdf), where the human body is modeled as a system of rigid segments in order to study whole-body mass-center motion.

This does **not** mean the real body is perfectly rigid.
It means a segmented abstraction is an accepted and useful starting point for locomotion analysis.

#### 3.1.2 The Quiet-Standing CoM Is Around 57% of Body Height

[The Motion of Body Center of Mass During Walking](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf) states that in quietly standing humans, the CoM lies at about **57% of body height from the ground**.

This value will be used in this document as the nominal standing reference for the project CM.

#### 3.1.3 Articulated Figures Need a Clear Hierarchical Structure

[Inverse Kinematics and Geometric Constraints for Articulated Figure Manipulation.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Inverse%20Kinematics%20and%20Geometric%20Constraints%20for%20Articulated%20Figure%20Manipulation.pdf) motivates the use of hierarchical articulated structures for figure manipulation and inverse kinematics.

This supports the need for a clearly defined body graph and parent-child hierarchy in the present stickman model.

### 3.2 Project-Specific Simplifications

The following elements are **design choices of this project**:

* the model is planar and 2D
* the torso is represented by only three nodes: `torso_bottom`, `torso_center`, `torso_top`
* there are no explicit `hips` or `shoulders`
* each arm has only two segments
* each leg has only two segments
* the head is represented as a circle
* the total standing height is defined through a five-unit vertical construction

These are not claims about true human anatomy.
They are simplifications chosen to keep the body model compact and useful for locomotion control.

---

## 4. Global Units and Reference Frame

The model uses:

* **length** in meters
* **mass** in kilograms
* a **2D world-space frame**

The standing reference frame is defined as follows:

* the model uses its own **world / simulation frame**
* in that frame, the ground is the zero-height reference
* in that frame, the vertical axis points upward
* the horizontal axis lies in the 2D locomotion plane
* the upright standing pose is the nominal geometric reference of the model

This means that every nominal body quantity in this document is defined relative to a standing stickman above a flat reference ground.

### 4.1 Important Distinction: Model Coordinates vs SDL Screen Coordinates

The geometric model and the SDL renderer do **not** have to use the same vertical convention.

In this document:

* the **model / simulation frame** uses the natural biomechanical convention: upward is positive and ground height is the zero reference

In SDL rendering:

* the **screen frame** typically uses pixel coordinates whose vertical axis grows downward

This is not a contradiction.
It simply means that the View will later need a coordinate conversion between:

* model-space positions
* screen-space positions

The stickman model itself should remain defined in a physically and geometrically readable world frame, while SDL is treated as a rendering convention.

---

## 5. Stickman Illustration and Node Naming

The reference illustration of the model is:

![Stickman node naming](../assets/stickman_nodes_name.png)

This figure is important because it fixes the intended visible topology of the stickman.
The document follows the node naming used in that illustration.

The visible structure is minimal:

* one head anchor
* three torso nodes
* two nodes per arm
* two nodes per leg

That is why the model has no explicit shoulders or hips.
The branching structure is represented directly through `torso_top` and `torso_bottom`.

---

## 6. Node Set

The explicit nodes of the stickman are:

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

### 6.1 Node Roles

The nodes can be grouped by function.

| Node | Role |
|---|---|
| `head_top` | head anchor / visual anchor |
| `torso_top` | upper torso branching node |
| `torso_center` | middle torso reference node |
| `torso_bottom` | lower torso branching node / structural root |
| `elbow_left`, `elbow_right` | upper limb intermediate nodes |
| `wrist_left`, `wrist_right` | upper limb end-effectors |
| `knee_left`, `knee_right` | lower limb intermediate nodes |
| `ankle_left`, `ankle_right` | lower limb end-effectors / support-relevant nodes |

This table is simple, but important.
It clarifies that not all nodes play the same role in the model.

---

## 7. Topology of the Stickman

The stickman graph is defined by the following direct connections:

* `head_top` <-> `torso_top`
* `torso_top` <-> `torso_center`
* `torso_center` <-> `torso_bottom`
* `torso_top` <-> `elbow_left` <-> `wrist_left`
* `torso_top` <-> `elbow_right` <-> `wrist_right`
* `torso_bottom` <-> `knee_left` <-> `ankle_left`
* `torso_bottom` <-> `knee_right` <-> `ankle_right`

This graph is a direct translation of the reference illustration, not of a full anatomical skeleton.

### 7.1 Why There Are No Hips or Shoulders

This is one of the easiest places to get confused, so it is worth stating clearly.

The model has no explicit hip nodes and no explicit shoulder nodes because:

* the project uses a **minimal visible node graph**
* the branching itself is represented by torso nodes
* the goal is to control and render a simplified stickman, not to reproduce every anatomical landmark

So:

* `torso_top` plays the role of the upper branching point
* `torso_bottom` plays the role of the lower branching point

This is a **design simplification**, not a biomechanical claim.

---

## 8. Hierarchical Structure

The articulated hierarchy of the model is:

```text
root (global transform)
 └── torso_bottom
      ├── torso_center
      │    └── torso_top
      │         ├── head_top
      │         ├── elbow_left
      │         │    └── wrist_left
      │         └── elbow_right
      │              └── wrist_right
      ├── knee_left
      │    └── ankle_left
      └── knee_right
           └── ankle_right
```

### 8.1 Technical Root vs Structural Root

The global `root` is a **technical transform**.
It is used to place the whole character in world space.
It is not a body node of the stickman itself.

### 8.2 Why `torso_bottom` Is the Structural Root

`torso_bottom` is chosen as the structural root of the body graph because:

* both legs originate from it
* the torso chain extends upward from it

This makes it the most natural **lower branching node** of the simplified body.

### 8.3 Why `torso_center` Is Still Important

Even though `torso_bottom` is the structural root, `torso_center` remains very important because, as shown later, it is the node closest to the nominal standing CM.

This distinction must be kept clear:

* `torso_bottom` is important topologically
* `torso_center` is important geometrically

---

## 9. Primary Geometric Parameters

The primary geometric parameters of the model are:

* `total_height = 1.80 m`
* `total_mass = 60 kg`
* `limb_rest_length = total_height / 5`

These are the parameters from which the basic scale of the stickman is derived.

### 9.1 Derived Limb Rest Length

By project definition, the stickman has a standing height of five equal vertical units:

```text
total_height = 5 * limb_rest_length
```

Therefore:

```text
limb_rest_length = total_height / 5
                 = 1.80 / 5
                 = 0.36 m
```

This is a directly derived result.

For convenience, the rest of the document will often denote:

```text
L_rest = limb_rest_length
```

So in this project:

```text
L_rest = 0.36 m
```

---

## 10. Geometric Vertical Structure

The nominal vertical chain of the stickman is:

```text
head_top
  |
torso_top
  |
torso_center
  |
torso_bottom
  |
knee
  |
ankle
```

Each adjacent level is separated by one `limb_rest_length`.

### 10.1 Nominal Heights in Limb Units

Because the total standing height is defined as five equal vertical units, the node heights in the upright reference configuration are:

* `ankle = 0.0 * L_rest`
* `knee = 1.0 * L_rest`
* `torso_bottom = 2.0 * L_rest`
* `torso_center = 3.0 * L_rest`
* `torso_top = 4.0 * L_rest`
* `head_top = 5.0 * L_rest`

This result is directly derived from the project definition.

### 10.2 Nominal Heights in Meters

Since `L_rest = 0.36 m`, the same heights become:

* `ankle = 0.00 m`
* `knee = 0.36 m`
* `torso_bottom = 0.72 m`
* `torso_center = 1.08 m`
* `torso_top = 1.44 m`
* `head_top = 1.80 m`

This table is useful later because it provides an immediate intuition for the real physical scale of the body.

---

## 11. Head Geometry

The head is represented as a circle attached to the top of the torso.

The nominal vertical extent of the head is defined by:

* bottom anchor at `torso_top`
* top anchor at `head_top`

If that segment is interpreted as the head diameter, then:

```text
head_diameter = limb_rest_length = L_rest
head_radius   = 0.5 * limb_rest_length = 0.5 * L_rest
```

Since `L_rest = 0.36 m`:

```text
head_radius = 0.18 m
```

This is again a directly derived geometric result.

---

## 12. Nominal Standing Configuration

The nominal standing configuration is the geometric reference pose of the stickman.

In this reference:

* the torso chain is vertically aligned
* the head is centered above `torso_top`
* the legs branch downward from `torso_bottom`
* the arms branch outward from `torso_top`
* the whole body is upright with respect to the world vertical axis

This configuration should be understood as a **reference geometry**, not yet as a locomotion controller state.

Its role is simply to define:

* the default scale of the body
* the default interpretation of the nodes
* the reference from which later motion and control quantities can be discussed

---

## 13. Nominal Center of Mass Reference

### 13.1 Literature Reference

According to [The Motion of Body Center of Mass During Walking](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf), the CoM of quietly standing humans lies at about **57% of body height from the ground**.

This project adopts that ratio as the nominal standing CM reference.

### 13.2 Derived CM Height

By definition:

```text
cm_height_standing = 0.57 * total_height
```

Since:

```text
total_height = 5 * L_rest
```

it follows that:

```text
cm_height_standing = 0.57 * 5 * L_rest
                   = 2.85 * L_rest
```

Since `L_rest = 0.36 m`:

```text
cm_height_standing = 2.85 * 0.36
                   = 1.026 m
```

This is a directly derived result once the `57%` literature reference is adopted.

### 13.3 Relative Position with Respect to Torso Nodes

From the nominal standing geometry:

* `torso_bottom = 2.0 * L_rest`
* `torso_center = 3.0 * L_rest`

Therefore:

```text
distance(CM, torso_bottom) = 2.85 * L_rest - 2.0 * L_rest = 0.85 * L_rest
distance(torso_center, CM) = 3.0 * L_rest - 2.85 * L_rest = 0.15 * L_rest
```

So the nominal standing CM lies:

* `0.85 * L_rest` above `torso_bottom`
* `0.15 * L_rest` below `torso_center`

This is why `torso_center` is the closest skeletal reference node to the nominal standing CM.

### 13.4 Important Clarification

The CM is **not** a skeletal node.
It is a world-space quantity associated with the locomotion model.

So:

* `torso_center` is the closest body node to the nominal standing CM
* `CM` itself still remains external to the articulated hierarchy

---

## 14. Structural Root vs Closest CM Reference Node

This distinction is important enough to deserve its own summary.

The model contains three different notions:

### 14.1 Technical Root

The global `root` is only a transform used to place the character in world space.

### 14.2 Structural Root

`torso_bottom` is the structural root of the simplified body graph because it is the lower branching node of the stickman.

### 14.3 Closest CM Reference Node

`torso_center` is the closest skeletal reference node to the nominal standing CM, because the adopted 57% CoM height places the CM at `2.85 * L_rest`, much closer to `3.0 * L_rest` than to `2.0 * L_rest`.

These three notions must not be confused:

* technical placement
* topological organization
* geometric proximity to the CM

---

## 15. End-Effectors and Contact-Relevant Nodes

The main end-effectors of the model are:

* `wrist_left`
* `wrist_right`
* `ankle_left`
* `ankle_right`

Among them, the most important for locomotion are:

* `ankle_left`
* `ankle_right`

These nodes are the main support-relevant endpoints of the current simplified model.

They will later be used for:

* stance definition
* support transitions
* contact reasoning
* foot placement planning

### 15.1 Important Note About Feet

At this stage of the project, the stickman does **not** yet define a full explicit foot segment.
However, this should not be interpreted as saying that support can remain forever a single ideal point.

For the first geometric model:

* `ankle_left` and `ankle_right` are the lower end-effectors

For future physical and terrain-aware extensions:

* each ankle may later need an associated support geometry or foot representation

This document therefore keeps the body minimal, while leaving room for later support refinement.

---

## 16. Constant-Length Assumptions

The nominal geometric model assumes fixed rest lengths for its main body links.

In particular:

* the vertical torso spacing follows `limb_rest_length`
* each arm is represented by two fixed-length links
* each leg is represented by two fixed-length links
* the head geometry is fixed relative to `torso_top` and `head_top`

This does **not** yet define how strongly these lengths will be enforced in later physical or procedural reconstruction.
It only defines the nominal geometric body.

---

## 17. What the Model Does Not Yet Contain

This document intentionally does **not** yet define:

* segment-wise mass distribution
* segment inertia values
* joint torque laws
* locomotion mode logic
* hybrid switching criteria
* physical perturbation thresholds
* contact solver behavior
* terrain adaptation logic
* curve or spline rendering rules
* software classes or module interfaces

Those topics belong to later design documents.

---

## 18. Summary of Proven, Assumed, and Deferred Elements

### 18.1 Directly Derived from Definitions

The following results are directly derived from the project definitions:

* `limb_rest_length = total_height / 5 = 0.36 m`
* nominal node heights along the vertical chain
* `head_radius = 0.5 * L_rest = 0.18 m` if the head diameter is taken as `L_rest`
* `cm_height_standing = 0.57 * total_height = 2.85 * L_rest = 1.026 m`
* the relative CM distances to `torso_bottom` and `torso_center`

### 18.2 Taken from Literature

The following assumptions are explicitly literature-backed:

* the body may be treated as a segmented system
* the quiet-standing CoM lies around 57% of body height
* articulated figures should have a clear hierarchical structure

Relevant sources:

* [Trajectory of the human body mass centre during walking at different speeds.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Trajectory%20of%20the%20human%20body%20mass%20centre%20during%20walking%20at%20different%20speeds.pdf)
* [The Motion of Body Center of Mass During Walking](/home/tomas/UL1/LIFAPCD/phy/doc/papers/The%20Motion%20of%20Body%20Center%20of%20Mass%20During%20Walking%3A%20A%20Review%20Oriented%20to%20Clinical%20Applications.pdf)
* [Inverse Kinematics and Geometric Constraints for Articulated Figure Manipulation.pdf](/home/tomas/UL1/LIFAPCD/phy/doc/papers/Inverse%20Kinematics%20and%20Geometric%20Constraints%20for%20Articulated%20Figure%20Manipulation.pdf)

### 18.3 Explicit Project Simplifications

The following are deliberate project simplifications:

* planar 2D stickman
* no explicit hips or shoulders
* minimal torso chain
* minimal arm and leg chains
* external CM variable
* topological simplification chosen for clarity and control

### 18.4 Deferred to Later Documents

The following topics are intentionally deferred:

* mass distribution
* locomotion control
* CM dynamics by locomotion regime
* recovery and falling logic
* physical pose tracking
* rendering spline construction
* MVC decomposition

This closes the present document and prepares the transition to the next design layers.
