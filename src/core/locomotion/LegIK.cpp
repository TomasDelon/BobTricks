#include "core/locomotion/LegIK.h"

#include <cmath>

LegIKResult computeKnee(const Vec2& P, const Vec2& F_target, double L, double facing)
{
    Vec2 F = F_target;
    // Distance pelvis → foot.
    const double dx    = F.x - P.x;
    const double dy    = F.y - P.y;
    double       d_leg = std::sqrt(dx * dx + dy * dy);

    // Clamp: avoid singularity at full extension.
    static constexpr double EPS = 1e-4;
    const double max_reach = 2.0 * L - EPS;
    if (d_leg > max_reach) {
        // Scale foot position back onto the reachable sphere.
        const double scale = max_reach / d_leg;
        F.x   = P.x + dx * scale;
        F.y   = P.y + dy * scale;
        d_leg = max_reach;
    }
    if (d_leg < EPS) {
        // Degenerate: pelvis on top of foot — place knee directly forward.
        return { { P.x + facing * L, P.y }, F };
    }

    // Height of knee above the midpoint of PF (isoceles triangle).
    // From: L² = (d/2)² + h²  →  h = sqrt(L² - (d/2)²)
    const double h_k = std::sqrt(std::max(0.0, L * L - (d_leg * 0.5) * (d_leg * 0.5)));

    // Unit vector along PF and its CCW perpendicular.
    const double ex = (F.x - P.x) / d_leg;
    const double ey = (F.y - P.y) / d_leg;

    // Perpendicular (CCW rotation of e_PF by 90°): n = (-ey, ex)
    double nx = -ey;
    double ny =  ex;

    // Knee bends forward: n must have a positive x-component in the facing direction.
    if (nx * facing < 0.0) {
        nx = -nx;
        ny = -ny;
    }

    // Midpoint of PF + offset along perpendicular.
    const Vec2 M    = { (P.x + F.x) * 0.5, (P.y + F.y) * 0.5 };
    const Vec2 knee = { M.x + h_k * nx, M.y + h_k * ny };
    return { knee, F };
}
