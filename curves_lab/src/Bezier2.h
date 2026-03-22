#pragma once

#include <utility>

#include "Curve2Common.h"

namespace curves {

struct QuadraticBezier2 {
    Vec2 b0 {};
    Vec2 b1 {};
    Vec2 b2 {};

    Vec2 evalByBernstein(double t) const;
    Vec2 evalByCasteljau(double t) const;
    Vec2 derivative(double t) const;
    Vec2 secondDerivative(double t) const;
    CurveSample2 sample(double t) const;
    std::pair<QuadraticBezier2, QuadraticBezier2> split(double t) const;
};

struct CubicBezier2 {
    Vec2 b0 {};
    Vec2 b1 {};
    Vec2 b2 {};
    Vec2 b3 {};

    Vec2 evalByBernstein(double t) const;
    Vec2 evalByCasteljau(double t) const;
    Vec2 derivative(double t) const;
    Vec2 secondDerivative(double t) const;
    CurveSample2 sample(double t) const;
    std::pair<CubicBezier2, CubicBezier2> split(double t) const;
};

} // namespace curves
