#include "core/character/CharacterState.h"
#include "core/locomotion/LegIK.h"
#include "core/math/MathConstants.h"

#include <algorithm>
#include <cmath>

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

    // ── État locomoteur ──────────────────────────────────────────────────────
    if (!on_floor) {
        ch.locomotion_state = LocomotionState::Airborne;
    } else if (run_mode) {
        ch.locomotion_state = LocomotionState::Running;
    } else if (std::abs(vx) > rc.walk_eps) {
        ch.locomotion_state = LocomotionState::Walking;
    } else {
        ch.locomotion_state = LocomotionState::Standing;
    }

    // ── Orientation ───────────────────────────────────────────────────────────
    if (vx >  rc.facing_eps) ch.facing =  1.0;
    else if (vx < -rc.facing_eps) ch.facing = -1.0;

    // ── Colonne du torse ──────────────────────────────────────────────────────
    // L'inclinaison combine :
    // - une inclinaison locomotrice vers la direction du mouvement ;
    // - une inclinaison vers l'amont induite par la pente.
    // En descente, les deux composantes peuvent se contrarier ; le torse reste
    // néanmoins orienté vers la pente si ce terme domine.
    const double L         = config.body_height_m / 5.0;
    const double d         = config.cm_pelvis_ratio * L;
    const double theta_max   = rc.theta_max_deg * kDegToRad;
    const double v_ref       = std::max(rc.v_ref, 1.0e-4);
    const double safe_slope  = std::isfinite(terrain_slope) ? terrain_slope : 0.0;
    const double tau_slope = (rc.tau_slope > 0.0) ? rc.tau_slope : dt;
    ch.filtered_slope += (safe_slope - ch.filtered_slope) * dt / tau_slope;
    const double slope_lp = ch.filtered_slope;

    const double move_sign   = (std::abs(vx) > rc.facing_eps)
                             ? std::copysign(1.0, vx)
                             : ch.facing;
    const double uphill_alignment = move_sign * slope_lp;
    double slope_lean_scale = 1.0;
    if (on_floor && run_mode && uphill_alignment > 0.0) {
        const double uphill_strength = std::clamp(uphill_alignment / 0.35, 0.0, 1.0);
        slope_lean_scale = std::lerp(1.0, 0.55, uphill_strength);
    }
    const double theta_slope = slope_lean_scale
                             * rc.slope_lean_factor
                             * std::atan(slope_lp);
    const double airborne_target = on_floor ? 0.0 : 1.0;
    const double airborne_tau = 0.08;
    ch.airborne_lean_blend += (airborne_target - ch.airborne_lean_blend)
                            * dt / airborne_tau;
    ch.airborne_lean_blend = std::clamp(ch.airborne_lean_blend, 0.0, 1.0);
    // En l'air, le torse doit légèrement repartir vers l'arrière, mais moins
    // fortement que l'inclinaison vers l'avant utilisée au sol.
    const double velocity_lean_dir = std::lerp(1.0, -0.45, ch.airborne_lean_blend);
    const double theta_vel   = velocity_lean_dir * move_sign * theta_max
                             * std::tanh(std::abs(vx) / v_ref);
    const double theta_tgt = theta_slope + theta_vel;
    const double tau  = (rc.tau_lean > 0.0) ? rc.tau_lean : dt;
    ch.theta += (theta_tgt - ch.theta) * dt / tau;
    const double theta = ch.theta;
    const double hunch_min = std::max(0.0, rc.hunch_min_deg);
    const double hunch_max = std::max(hunch_min, rc.hunch_max_deg);
    const double hunch_deg = std::clamp(rc.hunch_current_deg, hunch_min, hunch_max);
    const double hunch = hunch_deg * kDegToRad;
    const double hunch_sign = ch.facing;
    const double lower_theta = theta - hunch_sign * hunch;
    const double upper_theta = theta + hunch_sign * hunch;

    const Vec2 spine_dir{std::sin(theta), std::cos(theta)};
    const Vec2 lower_dir{std::sin(lower_theta), std::cos(lower_theta)};
    const Vec2 upper_dir{std::sin(upper_theta), std::cos(upper_theta)};

    ch.pelvis = cm.position - spine_dir * d;
    ch.torso_center = ch.pelvis + lower_dir * L;
    ch.torso_top = ch.torso_center + upper_dir * L;

    // ── Position des genoux (IK analytique) ──────────────────────────────────
    if (ch.feet_initialized) {
        const auto ikL    = computeKnee(ch.pelvis, ch.foot_left.pos,  L, ch.facing);
        const auto ikR    = computeKnee(ch.pelvis, ch.foot_right.pos, L, ch.facing);
        ch.knee_left      = ikL.knee;
        ch.knee_right     = ikR.knee;
    }
}
