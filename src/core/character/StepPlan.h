#pragma once

#include "core/math/Vec2.h"

struct StepPlan {
    bool   active         = false;
    bool   move_right     = false;    // true = pie derecho en vuelo
    Vec2   takeoff_pos    = {0.0, 0.0};  // posición del pie al inicio del swing (fija)
    double takeoff_ground = 0.0;         // terrain.height_at(takeoff) al despegue
    Vec2   land_target    = {0.0, 0.0};
    double t_start        = 0.0;      // sim_time al inicio del swing
    double duration       = 0.25;     // T_swing (s)
    double clearance      = 0.0;      // h_clear — altura máxima del arco (m)
};
