#include "core/simulation/GroundReference.h"

#include <algorithm>
#include <cmath>

#include "core/math/MathConstants.h"

namespace {

double terrainDistSqToPelvis(const Terrain& terrain, const Vec2& pelvis, double x)
{
    const double y  = terrain.height_at(x);
    const double dx = x - pelvis.x;
    const double dy = y - pelvis.y;
    return dx * dx + dy * dy;
}

bool terrainPointInsideDisk(const Terrain& terrain,
                            const Vec2& pelvis,
                            double radius,
                            double x)
{
    return terrainDistSqToPelvis(terrain, pelvis, x) <= radius * radius;
}

bool isInsideInterval(const Terrain& terrain,
                      const Vec2& pelvis,
                      double radius_sq,
                      double x)
{
    return terrainDistSqToPelvis(terrain, pelvis, x) <= radius_sq;
}

double slideTerrainEndpointX(const Terrain& terrain,
                             const Vec2& pelvis,
                             double radius,
                             double x_prev,
                             double x_target,
                             double x_anchor,
                             double alpha)
{
    double x_start = x_prev;
    if (!terrainPointInsideDisk(terrain, pelvis, radius, x_start)) {
        if (terrainPointInsideDisk(terrain, pelvis, radius, x_anchor))
            x_start = clampTerrainEndpointX(terrain, pelvis, radius, x_anchor, x_start);
        else
            return x_prev;
    }

    if (std::abs(x_target - x_start) <= kEpsLength)
        return x_start;

    const double x_step = x_start + (x_target - x_start) * alpha;
    if (terrainPointInsideDisk(terrain, pelvis, radius, x_step))
        return x_step;
    return clampTerrainEndpointX(terrain, pelvis, radius, x_start, x_step);
}

} // namespace

double clampTerrainEndpointX(const Terrain& terrain,
                             const Vec2& pelvis,
                             double radius,
                             double x_start,
                             double x_target)
{
    const double r_sq = radius * radius;

    if (!isInsideInterval(terrain, pelvis, r_sq, x_start)) return x_start;
    if (isInsideInterval(terrain, pelvis, r_sq, x_target)) return x_target;

    double x_valid = x_start;
    constexpr int coarse_steps = 32;
    for (int i = 1; i <= coarse_steps; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(coarse_steps);
        const double x = x_start + (x_target - x_start) * t;
        if (!isInsideInterval(terrain, pelvis, r_sq, x)) {
            double x_invalid = x;
            for (int j = 0; j < 28; ++j) {
                const double xm = 0.5 * (x_valid + x_invalid);
                if (isInsideInterval(terrain, pelvis, r_sq, xm)) x_valid = xm;
                else            x_invalid = xm;
            }
            return x_valid;
        }
        x_valid = x;
    }

    return x_valid;
}

GroundReferenceSample computeGroundReferenceSample(const Terrain& terrain,
                                                   const Vec2& pelvis,
                                                   double cm_x,
                                                   double facing,
                                                   double speed_x,
                                                   double L,
                                                   const TerrainSamplingConfig& ts,
                                                   double dt,
                                                   double left_prev_x,
                                                   double right_prev_x)
{
    const double x_back_target = cm_x - facing * (ts.w_back * L);
    const double x_fwd_target  = cm_x + facing * (ts.w_fwd * L + std::abs(speed_x) * ts.t_look);
    const double reach         = 2.0 * L;
    const double x_left_target = std::min(x_back_target, x_fwd_target);
    const double x_right_target = std::max(x_back_target, x_fwd_target);
    const double tau = std::max(ts.tau_slide, 1.0e-4);
    const double alpha = 1.0 - std::exp(-dt / tau);
    const double x_left = slideTerrainEndpointX(terrain, pelvis, reach,
                                                left_prev_x, x_left_target, cm_x, alpha);
    const double x_right = slideTerrainEndpointX(terrain, pelvis, reach,
                                                 right_prev_x, x_right_target, cm_x, alpha);
    const double x_back = (facing >= 0.0) ? x_left : x_right;
    const double x_fwd  = (facing >= 0.0) ? x_right : x_left;
    const Vec2   back   = { x_back, terrain.height_at(x_back) };
    const Vec2   fwd    = { x_fwd,  terrain.height_at(x_fwd) };
    const double span   = std::abs(fwd.x - back.x);
    const double slope  = (span > 1.0e-9)
                        ? facing * (fwd.y - back.y) / span
                        : 0.0;
    return {
        back,
        fwd,
        (back.y + fwd.y) * 0.5,
        slope
    };
}

GroundReferenceState updateGroundReference(CharacterState& ch,
                                           const Terrain& terrain,
                                           const CMState& cm,
                                           const Vec2& pelvis_ref,
                                           bool airborne_ref,
                                           const TerrainSamplingConfig& ts,
                                           double L,
                                           double dt)
{
    GroundReferenceState state;
    state.pelvis_ref = pelvis_ref;
    state.airborne_ref = airborne_ref;

    if (!ch.ground_reference_initialized || state.airborne_ref) {
        ch.ground_left_x = cm.position.x;
        ch.ground_right_x = cm.position.x;
        ch.ground_reference_initialized = true;
    }

    state.sample = computeGroundReferenceSample(terrain, state.pelvis_ref, cm.position.x,
                                                ch.facing, cm.velocity.x, L, ts, dt,
                                                ch.ground_left_x, ch.ground_right_x);
    ch.ground_left_x  = std::min(state.sample.back.x, state.sample.fwd.x);
    ch.ground_right_x = std::max(state.sample.back.x, state.sample.fwd.x);
    return state;
}
