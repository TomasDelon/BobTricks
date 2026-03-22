#pragma once

#include "Bezier2.h"

namespace curves {

struct CatmullRomSegment2 {
    Vec2   p0 {};
    Vec2   p1 {};
    Vec2   p2 {};
    Vec2   p3 {};
    double alpha = 0.5; // 0=uniform, 0.5=centripetal, 1=chordal

    Vec2 eval(double u) const;
    Vec2 derivative(double u) const;
    Vec2 secondDerivative(double u) const;
    CurveSample2 sample(double u) const;
    CubicBezier2 toCubicBezier() const;
};

} // namespace curves
