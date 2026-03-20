#pragma once

#include "core/math/Vec2.h"

// Runtime state of the Center of Mass.
// Position and velocity in world units (m, m/s).
struct CMState {
    Vec2 position     = {0.0, 0.0};
    Vec2 velocity     = {0.0, 0.0};
    Vec2 acceleration = {0.0, 0.0};  // net accel this step (m/s²), for display
};
