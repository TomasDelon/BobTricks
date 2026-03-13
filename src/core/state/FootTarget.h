#pragma once

#include "FootSide.h"
#include "../math/Vec2.h"

/// @brief Cible de balancement d'un pied pendant la phase de swing.
struct FootTarget {
    FootSide foot             = FootSide::Left;
    Vec2     takeoff_position = {};  ///< position au décollage
    Vec2     target_position  = {};  ///< position cible à l'atterrissage
    double   lift_height      = 0.0; ///< hauteur maximale de l'arc de balancement
};
