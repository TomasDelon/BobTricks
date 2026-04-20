#pragma once

#include "core/math/Vec2.h"

#include <cstdint>

struct DustParticle {
    double spawn_time = 0.0;
    double lifetime_s = 0.0;
    Vec2   pos        = {0.0, 0.0};
    Vec2   vel        = {0.0, 0.0};
    float  radius_px  = 1.0f;
    float  alpha      = 0.0f;
    float  stretch    = 1.0f;
    std::uint8_t color_r = 214;
    std::uint8_t color_g = 198;
    std::uint8_t color_b = 170;
};
