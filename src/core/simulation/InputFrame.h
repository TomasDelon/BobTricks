#pragma once

#include <optional>
#include "core/math/Vec2.h"

/**
 * @brief Entrées consommées par `SimulationCore` pendant un pas fixe.
 */
struct InputFrame {
    bool key_left  = false;
    bool key_right = false;
    bool key_run   = false;
    bool jump      = false;

    // Right-drag: overrides CM velocity at start of step.
    std::optional<Vec2> set_velocity;

    // Optional world-space gaze target used by upper-body kinematics.
    std::optional<Vec2> gaze_target_world;

    // Left-drag: drag feet to a world position.
    // SimulationCore applies the circle constraint after positioning.
    bool              foot_left_drag  = false;
    Vec2              foot_left_pos   = {0.0, 0.0};   // world target
    bool              foot_right_drag = false;
    Vec2              foot_right_pos  = {0.0, 0.0};   // world target

    // Left-drag: override hand IK target to a world position.
    bool              hand_left_drag  = false;
    Vec2              hand_left_pos   = {0.0, 0.0};   // world target
    bool              hand_right_drag = false;
    Vec2              hand_right_pos  = {0.0, 0.0};   // world target
};
