#pragma once

#include "core/math/Vec2.h"

/**
 * @brief Résultat d'une IK analytique de jambe à deux segments.
 */
struct LegIKResult {
    Vec2 knee;
    Vec2 foot_eff;  // equals F if reachable, clamped otherwise
};

/**
 * @brief Calcule la position du genou et du pied effectif pour une jambe.
 */
LegIKResult computeKnee(Vec2 P, Vec2 F, double L, double facing);
