#pragma once

#include <optional>

#include "core/math/Vec2.h"

/** @brief Cibles optionnelles injectées dans la cinématique du haut du corps. */
struct UpperBodyTargets {
    std::optional<Vec2> left_hand_target;
    std::optional<Vec2> right_hand_target;
    std::optional<Vec2> gaze_target_world;
};

/** @brief Entrées agrégées pour la mise à jour du haut du corps. */
struct UpperBodyControl {
    double input_dir = 0.0;
    UpperBodyTargets targets;
};
