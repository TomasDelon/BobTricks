#include "CatmullRom2.h"

#include <algorithm>

namespace curves {

namespace {

struct HermiteData2 {
    Vec2 p1 {};
    Vec2 p2 {};
    Vec2 m1 {};
    Vec2 m2 {};
};

double knotIncrement(const Vec2& a, const Vec2& b, double alpha)
{
    const double d = (b - a).length();
    return std::pow(std::max(d, 1e-9), alpha);
}

HermiteData2 buildHermite(const CatmullRomSegment2& seg)
{
    const double t0 = 0.0;
    const double t1 = t0 + knotIncrement(seg.p0, seg.p1, seg.alpha);
    const double t2 = t1 + knotIncrement(seg.p1, seg.p2, seg.alpha);
    const double t3 = t2 + knotIncrement(seg.p2, seg.p3, seg.alpha);

    const Vec2 m1 = (((seg.p1 - seg.p0) / (t1 - t0))
                  -  ((seg.p2 - seg.p0) / (t2 - t0))
                  +  ((seg.p2 - seg.p1) / (t2 - t1))) * (t2 - t1);

    const Vec2 m2 = (((seg.p2 - seg.p1) / (t2 - t1))
                  -  ((seg.p3 - seg.p1) / (t3 - t1))
                  +  ((seg.p3 - seg.p2) / (t3 - t2))) * (t2 - t1);

    return { seg.p1, seg.p2, m1, m2 };
}

} // namespace

Vec2 CatmullRomSegment2::eval(double u) const
{
    const auto h = buildHermite(*this);

    const double u2 = u * u;
    const double u3 = u2 * u;

    const double h00 =  2.0 * u3 - 3.0 * u2 + 1.0;
    const double h10 =        u3 - 2.0 * u2 + u;
    const double h01 = -2.0 * u3 + 3.0 * u2;
    const double h11 =        u3 -       u2;

    return h.p1 * h00 + h.m1 * h10 + h.p2 * h01 + h.m2 * h11;
}

Vec2 CatmullRomSegment2::derivative(double u) const
{
    const auto h = buildHermite(*this);

    const double u2 = u * u;

    const double h00 =  6.0 * u2 - 6.0 * u;
    const double h10 =  3.0 * u2 - 4.0 * u + 1.0;
    const double h01 = -6.0 * u2 + 6.0 * u;
    const double h11 =  3.0 * u2 - 2.0 * u;

    return h.p1 * h00 + h.m1 * h10 + h.p2 * h01 + h.m2 * h11;
}

Vec2 CatmullRomSegment2::secondDerivative(double u) const
{
    const auto h = buildHermite(*this);

    const double h00 =  12.0 * u - 6.0;
    const double h10 =   6.0 * u - 4.0;
    const double h01 = -12.0 * u + 6.0;
    const double h11 =   6.0 * u - 2.0;

    return h.p1 * h00 + h.m1 * h10 + h.p2 * h01 + h.m2 * h11;
}

CurveSample2 CatmullRomSegment2::sample(double u) const
{
    return makeSample(eval(u), derivative(u), secondDerivative(u));
}

CubicBezier2 CatmullRomSegment2::toCubicBezier() const
{
    const auto h = buildHermite(*this);
    return {
        h.p1,
        h.p1 + h.m1 / 3.0,
        h.p2 - h.m2 / 3.0,
        h.p2
    };
}

} // namespace curves
