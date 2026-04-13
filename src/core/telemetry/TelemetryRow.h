#pragma once

#include "core/character/CharacterState.h"   // LocomotionState

/** @brief Ligne élémentaire de télémétrie exportée en CSV. */
struct TelemetryRow {
    double          t;
    double          cm_x,     cm_vx;
    double          cm_y,     cm_vy;
    double          pelvis_x;
    LocomotionState loco_state;
    double          foot_L_x, foot_R_x;
    double          foot_L_y, foot_R_y;
    bool            foot_L_on_ground, foot_R_on_ground;
    double          cm_target_y;
    double          ref_ground;
    double          ref_slope;
    double          h_ip;
    double          cm_offset;
    double          speed_drop;
    double          slope_drop;

    // Legacy compatibility field kept for older scenario assertions.
    // The current walking-redesign runtime no longer emits heel-strike events here.
    bool            heel_strike  = false;
};
