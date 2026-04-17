#pragma once

#include <cmath>
#include "core/math/MathConstants.h"

/**
 * @brief Vecteur 2D minimal utilisé dans tout le noyau.
 */
struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(double s)      const { return {x * s,   y * s};   }
    Vec2 operator/(double s)      const { return {x / s,   y / s};   }

    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }

    double length() const { return std::sqrt(x * x + y * y); }
};

inline double dot(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

inline Vec2 normalizeOr(Vec2 v, Vec2 fallback)
{
    const double len = v.length();
    if (len <= kEpsLength) return fallback;
    return v / len;
}
