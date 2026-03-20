#pragma once

#include "core/math/Vec2.h"

enum class FootPhase { Planted, Swing, Takeoff, Landing };

struct FootState {
    FootPhase phase     = FootPhase::Planted;
    Vec2      pos       = {0.0, 0.0};  // posición actual del tobillo (mundo)
    double    ground_y  = 0.0;         // terrain.height_at(pos.x) al plantar
    Vec2      target    = {0.0, 0.0};  // target del swing (fijado al inicio)
    double    swing_t0  = 0.0;         // sim_time al inicio del swing
    double    swing_T   = 0.25;        // duración prevista del swing (s)
};
