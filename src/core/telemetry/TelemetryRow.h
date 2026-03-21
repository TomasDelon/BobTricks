#pragma once

#include "core/locomotion/StepTriggerType.h"
#include "core/character/CharacterState.h"   // LocomotionState

// One row of simulation telemetry — one per sim tick.
//
// CSV format version 2 (18 columns — added heel_strike vs v1's 17).
// Column order is frozen within a version; do not reorder without bumping version.
//
//   t, cm_x, cm_vx, cm_y, cm_vy,
//   pelvis_x,
//   xcom, mos,
//   support_left, support_right, support_width,
//   foot_L_x, foot_R_x,
//   step_active, swing_right,
//   trigger,
//   heel_strike,
//   loco_state
//
// Three distinct locomotion events per tick (NOT mutually exclusive —
// heel_strike and trigger can both be 1 in the same row when a step lands
// and a new one is immediately triggered in the same tick):
//   trigger     != None  → shouldStep() fired this tick (even if planStep() failed)
//   step_active == 1     → a step was successfully planned and is in flight
//   heel_strike == 1     → a swing foot landed this tick
//
// swing_right is always 0 when step_active == 0 (no stale values).
struct TelemetryRow {
    double          t;
    double          cm_x,         cm_vx;
    double          cm_y,         cm_vy;
    double          pelvis_x;
    double          xcom,         mos;
    double          support_left, support_right, support_width;
    double          foot_L_x,     foot_R_x;
    bool            step_active;
    bool            swing_right;   // only meaningful when step_active == true
    StepTriggerType trigger;
    bool            heel_strike;
    LocomotionState loco_state;
};
