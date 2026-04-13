#pragma once

#include "core/math/Vec2.h"

/** @brief État complet d'un pied: contact, pinning et swing. */
struct FootState {
    Vec2 pos           = {0.0, 0.0};
    bool on_ground     = false;
    Vec2 ground_normal = {0.0, 1.0};  // outward terrain normal at contact point

    // Pinned: foot is anchored to pinned_pos (second priority after 2L circle).
    // Toggle via right-click.
    bool pinned        = false;
    Vec2 pinned_pos    = {0.0, 0.0};
    Vec2 pinned_normal = {0.0, 1.0};  // terrain normal frozen at pin time

    // Swing arc: foot travels from swing_start to swing_target over swing_t [0,1].
    // While swinging, pinned=false; on landing (swing_t>=1) foot auto-pins.
    bool   swinging          = false;
    Vec2   swing_start       = {0.0, 0.0};
    Vec2   swing_target      = {0.0, 0.0};
    double swing_t           = 0.0;

    // Computed once at step initiation; used throughout the swing.
    // swing_speed_scale < 1 slows the arc on steep steps (more vertical travel).
    // swing_h_clear is the parabolic lift height for this specific step.
    double swing_speed_scale = 1.0;   // [0-1] slow-down factor for steep arcs
    double swing_h_clear     = 0.0;   // [m]   parabolic lift height
};
