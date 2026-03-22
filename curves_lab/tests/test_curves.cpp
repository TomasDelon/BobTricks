#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "ArcLength.h"
#include "Bezier2.h"
#include "CatmullRom2.h"

using namespace curves;

static void require(bool cond, const char* msg)
{
    if (!cond) {
        std::cerr << "FAIL: " << msg << "\n";
        std::exit(1);
    }
}

static void requireNear(double a, double b, double eps, const char* msg)
{
    require(std::abs(a - b) <= eps, msg);
}

static void requireNear(const Vec2& a, const Vec2& b, double eps, const char* msg)
{
    require(nearlyEqual(a, b, eps), msg);
}

static void testQuadraticEndpoints()
{
    QuadraticBezier2 q{{0, 0}, {1, 2}, {3, 4}};
    requireNear(q.evalByBernstein(0.0), q.b0, 1e-12, "quadratic B(0)=b0");
    requireNear(q.evalByBernstein(1.0), q.b2, 1e-12, "quadratic B(1)=b2");
    requireNear(q.derivative(0.0), (q.b1 - q.b0) * 2.0, 1e-12, "quadratic dB(0)");
    requireNear(q.derivative(1.0), (q.b2 - q.b1) * 2.0, 1e-12, "quadratic dB(1)");
}

static void testCubicBernsteinMatchesCasteljau()
{
    CubicBezier2 c{{0, 0}, {1, 2}, {2, -1}, {4, 3}};
    for (int i = 0; i <= 100; ++i) {
        const double t = static_cast<double>(i) / 100.0;
        requireNear(c.evalByBernstein(t), c.evalByCasteljau(t), 1e-12,
                    "cubic Bernstein == Casteljau");
    }
}

static void testCubicDerivativeEndpoints()
{
    CubicBezier2 c{{0, 0}, {1, 2}, {2, -1}, {4, 3}};
    requireNear(c.derivative(0.0), (c.b1 - c.b0) * 3.0, 1e-12, "cubic dB(0)");
    requireNear(c.derivative(1.0), (c.b3 - c.b2) * 3.0, 1e-12, "cubic dB(1)");
}

static void testBezierSubdivision()
{
    CubicBezier2 c{{0, 0}, {1, 2}, {2, -1}, {4, 3}};
    const double t = 0.37;
    const auto [left, right] = c.split(t);
    const Vec2 mid = c.evalByCasteljau(t);
    requireNear(left.b3, mid, 1e-12, "split left endpoint");
    requireNear(right.b0, mid, 1e-12, "split right endpoint");
}

static void testStraightLineArcLength()
{
    CubicBezier2 c{{0, 0}, {1, 0}, {2, 0}, {3, 0}};
    ArcLengthLUT2 lut = ArcLengthLUT2::build(
        [&](double t){ return c.derivative(t); }, 129, 8);

    requireNear(lut.totalLength(), 3.0, 1e-9, "straight line exact length");
    requireNear(lut.parameterAtArcLength(1.5), 0.5, 1e-9, "straight line s->t");
    requireNear(lut.arcLengthAtParameter(0.5), 1.5, 1e-9, "straight line t->s");
}

static void testArcLengthMonotone()
{
    CubicBezier2 c{{0, 0}, {1, 3}, {2, -1}, {4, 2}};
    ArcLengthLUT2 lut = ArcLengthLUT2::build(
        [&](double t){ return c.derivative(t); }, 129, 16);

    const auto& s = lut.arcLengths();
    const auto& p = lut.parameters();
    require(!s.empty(), "arc-length LUT not empty");

    for (std::size_t i = 1; i < s.size(); ++i) {
        require(s[i] >= s[i - 1], "arc lengths monotone");
        require(p[i] > p[i - 1], "parameters strictly increasing");
    }

    const std::vector<double> uniform = lut.uniformParametersByArcLength(16);
    for (std::size_t i = 1; i < uniform.size(); ++i) {
        require(uniform[i] >= uniform[i - 1], "uniform arc params monotone");
    }
}

static void testCatmullRomInterpolation()
{
    CatmullRomSegment2 s{{0, 0}, {1, 1}, {3, 1}, {4, 0}, 0.5};
    requireNear(s.eval(0.0), s.p1, 1e-12, "catmull rom C(0)=p1");
    requireNear(s.eval(1.0), s.p2, 1e-12, "catmull rom C(1)=p2");

    const CubicBezier2 b = s.toCubicBezier();
    requireNear(b.b0, s.p1, 1e-12, "catmull->bezier start");
    requireNear(b.b3, s.p2, 1e-12, "catmull->bezier end");
}

static void testCatmullRomDegenerateNoNan()
{
    CatmullRomSegment2 s{{0, 0}, {0, 0}, {1, 0}, {2, 0}, 0.5};
    for (int i = 0; i <= 32; ++i) {
        const double u = static_cast<double>(i) / 32.0;
        const CurveSample2 sample = s.sample(u);
        require(isFinite(sample.pos), "catmull sample finite");
        require(isFinite(sample.d1), "catmull derivative finite");
        require(isFinite(sample.d2), "catmull second derivative finite");
        require(std::isfinite(sample.curvature), "catmull curvature finite");
    }
}

int main()
{
    testQuadraticEndpoints();
    testCubicBernsteinMatchesCasteljau();
    testCubicDerivativeEndpoints();
    testBezierSubdivision();
    testStraightLineArcLength();
    testArcLengthMonotone();
    testCatmullRomInterpolation();
    testCatmullRomDegenerateNoNan();

    std::cout << "PASS curves_lab tests\n";
    return 0;
}
