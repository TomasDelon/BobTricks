#pragma once

#include "core/math/Vec2.h"

/** @brief Échantillon horodaté de la trajectoire du centre de masse. */
struct TrailPoint {
    double time;
    Vec2   pos;
};
