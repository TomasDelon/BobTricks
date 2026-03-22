#include "ArcLength.h"

#include <algorithm>

namespace curves {

namespace {

double gaussLegendre5(const std::function<Vec2(double)>& derivative,
                      double a,
                      double b)
{
    static constexpr double x[5] = {
        0.0,
       -0.5384693101056831,
        0.5384693101056831,
       -0.9061798459386640,
        0.9061798459386640
    };

    static constexpr double w[5] = {
        0.5688888888888889,
        0.4786286704993665,
        0.4786286704993665,
        0.2369268850561891,
        0.2369268850561891
    };

    const double c1 = 0.5 * (b - a);
    const double c2 = 0.5 * (b + a);

    double sum = 0.0;
    for (int i = 0; i < 5; ++i) {
        const double t = c1 * x[i] + c2;
        sum += w[i] * derivative(t).length();
    }
    return c1 * sum;
}

} // namespace

double integrateArcLength(const std::function<Vec2(double)>& derivative,
                          double a,
                          double b,
                          int subdivisions)
{
    subdivisions = std::max(subdivisions, 1);

    double total = 0.0;
    const double h = (b - a) / static_cast<double>(subdivisions);
    for (int i = 0; i < subdivisions; ++i) {
        const double x0 = a + h * static_cast<double>(i);
        const double x1 = x0 + h;
        total += gaussLegendre5(derivative, x0, x1);
    }
    return total;
}

ArcLengthLUT2 ArcLengthLUT2::build(const std::function<Vec2(double)>& derivative,
                                   int lut_samples,
                                   int quadrature_subdivisions)
{
    ArcLengthLUT2 lut;
    lut_samples = std::max(lut_samples, 2);

    lut.parameters_.resize(static_cast<std::size_t>(lut_samples));
    lut.arc_lengths_.resize(static_cast<std::size_t>(lut_samples));

    lut.parameters_[0] = 0.0;
    lut.arc_lengths_[0] = 0.0;

    for (int i = 1; i < lut_samples; ++i) {
        const double t0 = static_cast<double>(i - 1) / static_cast<double>(lut_samples - 1);
        const double t1 = static_cast<double>(i)     / static_cast<double>(lut_samples - 1);
        lut.parameters_[static_cast<std::size_t>(i)] = t1;
        lut.arc_lengths_[static_cast<std::size_t>(i)] =
            lut.arc_lengths_[static_cast<std::size_t>(i - 1)]
            + integrateArcLength(derivative, t0, t1, quadrature_subdivisions);
    }

    lut.total_length_ = lut.arc_lengths_.back();
    return lut;
}

double ArcLengthLUT2::parameterAtArcLength(double s) const
{
    if (parameters_.empty()) return 0.0;
    if (s <= 0.0) return 0.0;
    if (s >= total_length_) return 1.0;

    const auto it = std::lower_bound(arc_lengths_.begin(), arc_lengths_.end(), s);
    const std::size_t hi = static_cast<std::size_t>(it - arc_lengths_.begin());
    const std::size_t lo = hi - 1;

    const double s0 = arc_lengths_[lo];
    const double s1 = arc_lengths_[hi];
    const double t0 = parameters_[lo];
    const double t1 = parameters_[hi];

    if (s1 <= s0 + 1e-15) return t0;
    const double u = (s - s0) / (s1 - s0);
    return t0 + (t1 - t0) * u;
}

double ArcLengthLUT2::parameterAtFraction(double u) const
{
    if (u <= 0.0) return 0.0;
    if (u >= 1.0) return 1.0;
    return parameterAtArcLength(u * total_length_);
}

double ArcLengthLUT2::arcLengthAtParameter(double t) const
{
    if (parameters_.empty()) return 0.0;
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return total_length_;

    const auto it = std::lower_bound(parameters_.begin(), parameters_.end(), t);
    const std::size_t hi = static_cast<std::size_t>(it - parameters_.begin());
    const std::size_t lo = hi - 1;

    const double t0 = parameters_[lo];
    const double t1 = parameters_[hi];
    const double s0 = arc_lengths_[lo];
    const double s1 = arc_lengths_[hi];

    if (t1 <= t0 + 1e-15) return s0;
    const double u = (t - t0) / (t1 - t0);
    return s0 + (s1 - s0) * u;
}

std::vector<double> ArcLengthLUT2::uniformParametersByArcLength(int count) const
{
    std::vector<double> out;
    if (count <= 0) return out;

    out.reserve(static_cast<std::size_t>(count));
    if (count == 1) {
        out.push_back(0.0);
        return out;
    }

    for (int i = 0; i < count; ++i) {
        const double u = static_cast<double>(i) / static_cast<double>(count - 1);
        out.push_back(parameterAtFraction(u));
    }
    return out;
}

} // namespace curves
