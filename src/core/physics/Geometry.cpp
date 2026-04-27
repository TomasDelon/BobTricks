#include "core/physics/Geometry.h"

#include <algorithm>
#include <cmath>

double computeNominalY(double L, double d_pref, double cm_pelvis_ratio)
{
    const double half_d   = d_pref * L * 0.5;
    const double two_L    = 2.0 * L;
    const double h_pelvis = std::sqrt(std::max(0.0, two_L * two_L - half_d * half_d));
    return h_pelvis + cm_pelvis_ratio * L;
}
