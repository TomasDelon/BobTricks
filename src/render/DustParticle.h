#pragma once

#include "core/math/Vec2.h"

struct DustParticle {
    double spawn_time = 0.0;
    double lifetime_s = 0.0;
    Vec2   pos        = {0.0, 0.0};
    Vec2   vel        = {0.0, 0.0};
    float  radius_px  = 1.0f;
    float  alpha      = 0.0f;
};
