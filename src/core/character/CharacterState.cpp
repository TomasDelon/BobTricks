#include "core/character/CharacterState.h"
#include "core/locomotion/LegIK.h"

#include <cmath>

static constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;

void updateCharacterState(CharacterState& ch,
                          const CMState&  cm,
                          const CharacterConfig& config,
                          const CharacterReconstructionConfig& rc,
                          bool   on_floor,
                          bool   run_mode,
                          double dt,
                          double terrain_slope)
{
    const double vx = cm.velocity.x;

    // ── Locomotion state ─────────────────────────────────────────────────────
    if (!on_floor) {
        ch.locomotion_state = LocomotionState::Airborne;
    } else if (run_mode) {
        ch.locomotion_state = LocomotionState::Running;
    } else if (std::abs(vx) > rc.walk_eps) {
        ch.locomotion_state = LocomotionState::Walking;
    } else {
        ch.locomotion_state = LocomotionState::Standing;
    }

    // ── Facing ────────────────────────────────────────────────────────────────
    if (vx >  rc.facing_eps) ch.facing =  1.0;
    else if (vx < -rc.facing_eps) ch.facing = -1.0;

    // ── Torso spine ───────────────────────────────────────────────────────────
    // Lean combines:
    // - a locomotion lean towards movement direction
    // - an uphill lean from terrain slope
    // When descending, both components naturally oppose each other, so the
    // torso still tilts toward the hill if the slope term dominates.
    const double L         = config.body_height_m / 5.0;
    const double d         = config.cm_pelvis_ratio * L;
    const double theta_max   = rc.theta_max_deg * DEG_TO_RAD;
    const double v_ref       = std::max(rc.v_ref, 1.0e-4);
    const double safe_slope  = std::isfinite(terrain_slope) ? terrain_slope : 0.0;
    const double tau_slope = (rc.tau_slope > 0.0) ? rc.tau_slope : dt;
    ch.filtered_slope += (safe_slope - ch.filtered_slope) * dt / tau_slope;
    const double slope_lp = ch.filtered_slope;

    const double theta_slope = rc.slope_lean_factor * std::atan(slope_lp);
    const double move_sign   = (std::abs(vx) > rc.facing_eps)
                             ? std::copysign(1.0, vx)
                             : ch.facing;
    const double theta_vel   = move_sign * theta_max
                             * std::tanh(std::abs(vx) / v_ref);
    const double theta_tgt = theta_slope + theta_vel;
    const double tau  = (rc.tau_lean > 0.0) ? rc.tau_lean : dt;
    ch.theta += (theta_tgt - ch.theta) * dt / tau;
    const double theta = ch.theta;

    const double dx = std::sin(theta);
    const double dy = std::cos(theta);
    ch.pelvis.x       = cm.position.x - d         * dx;
    ch.pelvis.y       = cm.position.y - d         * dy;
    ch.torso_center.x = ch.pelvis.x   + L         * dx;
    ch.torso_center.y = ch.pelvis.y   + L         * dy;
    ch.torso_top.x    = ch.pelvis.x   + 2.0 * L   * dx;
    ch.torso_top.y    = ch.pelvis.y   + 2.0 * L   * dy;

    // ── Knee positions (analytic IK) ──────────────────────────────────────────
    if (ch.feet_initialized) {
        const auto ikL    = computeKnee(ch.pelvis, ch.foot_left.pos,  L, ch.facing);
        const auto ikR    = computeKnee(ch.pelvis, ch.foot_right.pos, L, ch.facing);
        ch.knee_left      = ikL.knee;
        ch.knee_right     = ikR.knee;
    }
}
