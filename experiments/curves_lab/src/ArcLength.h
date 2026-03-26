#pragma once

#include <functional>
#include <vector>

#include "Curve2Common.h"

namespace curves {

class ArcLengthLUT2 {
public:
    ArcLengthLUT2() = default;

    static ArcLengthLUT2 build(const std::function<Vec2(double)>& derivative,
                               int lut_samples = 129,
                               int quadrature_subdivisions = 128);

    double totalLength() const { return total_length_; }
    double parameterAtArcLength(double s) const;
    double parameterAtFraction(double u) const;
    double arcLengthAtParameter(double t) const;
    std::vector<double> uniformParametersByArcLength(int count) const;

    const std::vector<double>& parameters() const { return parameters_; }
    const std::vector<double>& arcLengths() const { return arc_lengths_; }

private:
    std::vector<double> parameters_;
    std::vector<double> arc_lengths_;
    double total_length_ = 0.0;
};

double integrateArcLength(const std::function<Vec2(double)>& derivative,
                          double a = 0.0,
                          double b = 1.0,
                          int subdivisions = 128);

} // namespace curves
