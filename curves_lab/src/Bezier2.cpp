#include "Bezier2.h"

namespace curves {

Vec2 QuadraticBezier2::evalByBernstein(double t) const
{
    const double u = 1.0 - t;
    return b0 * (u * u)
         + b1 * (2.0 * u * t)
         + b2 * (t * t);
}

Vec2 QuadraticBezier2::evalByCasteljau(double t) const
{
    const Vec2 q0 = lerp(b0, b1, t);
    const Vec2 q1 = lerp(b1, b2, t);
    return lerp(q0, q1, t);
}

Vec2 QuadraticBezier2::derivative(double t) const
{
    const double u = 1.0 - t;
    return (b1 - b0) * (2.0 * u)
         + (b2 - b1) * (2.0 * t);
}

Vec2 QuadraticBezier2::secondDerivative(double) const
{
    return (b2 - b1 * 2.0 + b0) * 2.0;
}

CurveSample2 QuadraticBezier2::sample(double t) const
{
    return makeSample(evalByBernstein(t), derivative(t), secondDerivative(t));
}

std::pair<QuadraticBezier2, QuadraticBezier2> QuadraticBezier2::split(double t) const
{
    const Vec2 q0 = lerp(b0, b1, t);
    const Vec2 q1 = lerp(b1, b2, t);
    const Vec2 r  = lerp(q0, q1, t);

    return {
        { b0, q0, r },
        { r, q1, b2 }
    };
}

Vec2 CubicBezier2::evalByBernstein(double t) const
{
    const double u  = 1.0 - t;
    const double u2 = u * u;
    const double u3 = u2 * u;
    const double t2 = t * t;
    const double t3 = t2 * t;

    return b0 * u3
         + b1 * (3.0 * u2 * t)
         + b2 * (3.0 * u * t2)
         + b3 * t3;
}

Vec2 CubicBezier2::evalByCasteljau(double t) const
{
    const Vec2 q0 = lerp(b0, b1, t);
    const Vec2 q1 = lerp(b1, b2, t);
    const Vec2 q2 = lerp(b2, b3, t);
    const Vec2 r0 = lerp(q0, q1, t);
    const Vec2 r1 = lerp(q1, q2, t);
    return lerp(r0, r1, t);
}

Vec2 CubicBezier2::derivative(double t) const
{
    const double u = 1.0 - t;
    return (b1 - b0) * (3.0 * u * u)
         + (b2 - b1) * (6.0 * u * t)
         + (b3 - b2) * (3.0 * t * t);
}

Vec2 CubicBezier2::secondDerivative(double t) const
{
    const double u = 1.0 - t;
    return (b2 - b1 * 2.0 + b0) * (6.0 * u)
         + (b3 - b2 * 2.0 + b1) * (6.0 * t);
}

CurveSample2 CubicBezier2::sample(double t) const
{
    return makeSample(evalByBernstein(t), derivative(t), secondDerivative(t));
}

std::pair<CubicBezier2, CubicBezier2> CubicBezier2::split(double t) const
{
    const Vec2 q0 = lerp(b0, b1, t);
    const Vec2 q1 = lerp(b1, b2, t);
    const Vec2 q2 = lerp(b2, b3, t);
    const Vec2 r0 = lerp(q0, q1, t);
    const Vec2 r1 = lerp(q1, q2, t);
    const Vec2 s  = lerp(r0, r1, t);

    return {
        { b0, q0, r0, s },
        { s,  r1, q2, b3 }
    };
}

} // namespace curves
