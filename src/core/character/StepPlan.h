#pragma once

#include "core/math/Vec2.h"

/** @brief Plan de pas élémentaire utilisé par les anciennes interfaces de marche. */
struct StepPlan {
    bool   active       = false;
    bool   move_right   = false;     // true = pie derecho en vuelo
    Vec2   takeoff_pos  = {0.0, 0.0};
    Vec2   land_target  = {0.0, 0.0};
    double swing_t      = 0.0;       // progreso del swing [0, 1], avanza a step_speed × dt
};
