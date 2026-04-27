#include "core/math/Bezier.h"

namespace {

double clampUnit(double t)
{
    if (t < 0.0) return 0.0;
    if (t > 1.0) return 1.0;
    return t;
}

} // fin namespace

Vec2 BezierQuadratic::eval(double t) const
{
    t = clampUnit(t);
    const double u = 1.0 - t;
    return p0 * (u * u) + p1 * (2.0 * u * t) + p2 * (t * t);
}

Vec2 BezierQuadratic::tangent(double t) const
{
    t = clampUnit(t);
    return (p1 - p0) * (2.0 * (1.0 - t)) + (p2 - p1) * (2.0 * t);
}

Vec2 BezierCubic::eval(double t) const
{
    t = clampUnit(t);
    const double u = 1.0 - t;
    const double uu = u * u;
    const double tt = t * t;
    return p0 * (uu * u)
         + p1 * (3.0 * uu * t)
         + p2 * (3.0 * u * tt)
         + p3 * (tt * t);
}

Vec2 BezierCubic::tangent(double t) const
{
    t = clampUnit(t);
    const double u = 1.0 - t;
    return (p1 - p0) * (3.0 * u * u)
         + (p2 - p1) * (6.0 * u * t)
         + (p3 - p2) * (3.0 * t * t);
}
