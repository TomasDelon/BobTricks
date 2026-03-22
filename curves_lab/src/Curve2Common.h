#pragma once

#include <cmath>
#include <algorithm>

#include "core/math/Vec2.h"

namespace curves {

inline double dot(const Vec2& a, const Vec2& b)
{
    return a.x * b.x + a.y * b.y;
}

inline double cross(const Vec2& a, const Vec2& b)
{
    return a.x * b.y - a.y * b.x;
}

inline double sqrLength(const Vec2& v)
{
    return dot(v, v);
}

inline Vec2 lerp(const Vec2& a, const Vec2& b, double t)
{
    return a * (1.0 - t) + b * t;
}

inline Vec2 normalizedOrZero(const Vec2& v)
{
    const double len = v.length();
    if (len <= 1e-15) return {};
    return v / len;
}

inline bool nearlyEqual(double a, double b, double eps = 1e-9)
{
    return std::abs(a - b) <= eps;
}

inline bool nearlyEqual(const Vec2& a, const Vec2& b, double eps = 1e-9)
{
    return nearlyEqual(a.x, b.x, eps) && nearlyEqual(a.y, b.y, eps);
}

inline bool isFinite(const Vec2& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y);
}

struct CurveSample2 {
    Vec2   pos {};
    Vec2   d1  {};
    Vec2   d2  {};
    double speed     = 0.0;
    double curvature = 0.0;
};

inline CurveSample2 makeSample(const Vec2& pos, const Vec2& d1, const Vec2& d2)
{
    CurveSample2 s;
    s.pos   = pos;
    s.d1    = d1;
    s.d2    = d2;
    s.speed = d1.length();

    if (s.speed > 1e-12) {
        s.curvature = cross(d1, d2) / std::pow(s.speed, 3.0);
    }
    return s;
}

} // namespace curves
