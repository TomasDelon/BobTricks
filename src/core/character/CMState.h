#pragma once

#include "core/math/Vec2.h"

/** @brief État cinématique courant du centre de masse. */
struct CMState {
    Vec2 position     = {0.0, 0.0};
    Vec2 velocity     = {0.0, 0.0};
    Vec2 acceleration = {0.0, 0.0};  // net accel this step (m/s²), for display
};
