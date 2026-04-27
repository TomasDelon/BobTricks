#include "core/math/Vec2.h"

#include <cmath>

Vec2 Vec2::operator+(const Vec2& o) const
{
    return {x + o.x, y + o.y};
}

Vec2 Vec2::operator-(const Vec2& o) const
{
    return {x - o.x, y - o.y};
}

Vec2 Vec2::operator*(double s) const
{
    return {x * s, y * s};
}

Vec2 Vec2::operator/(double s) const
{
    return {x / s, y / s};
}

Vec2& Vec2::operator+=(const Vec2& o)
{
    x += o.x;
    y += o.y;
    return *this;
}

Vec2& Vec2::operator-=(const Vec2& o)
{
    x -= o.x;
    y -= o.y;
    return *this;
}

double Vec2::length() const
{
    return std::sqrt(x * x + y * y);
}

double dot(const Vec2& a, const Vec2& b)
{
    return a.x * b.x + a.y * b.y;
}

Vec2 normalizeOr(const Vec2& v, const Vec2& fallback)
{
    const double len = v.length();
    if (len <= kEpsLength) return fallback;
    return v / len;
}
