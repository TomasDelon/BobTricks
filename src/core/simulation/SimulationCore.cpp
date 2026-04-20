#include "core/simulation/SimulationCore.h"

#include "core/character/CharacterState.h"
#include "core/character/ArmController.h"
#include "core/character/HeadController.h"
#include "core/character/UpperBodyKinematics.h"
#include "core/math/MathConstants.h"
#include "core/physics/Geometry.h"
#include "core/simulation/GroundReference.h"
#include "core/simulation/SimVerbosity.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#define SIM_LOG(...) do { if (g_sim_verbose) std::fprintf(stderr, __VA_ARGS__); } while (0)

static constexpr double kInputDirDeadzone = 0.01;

namespace {

static constexpr double kGroundContactSnapEps = 0.015;  // m

void applyGroundConstraint(FootState& foot, const Terrain& terrain);

Vec2 reconstructPelvis(const CMState& cm,
                       const CharacterConfig& cfg,
                       const CharacterReconstructionConfig& rcfg)
{
    const double L     = cfg.body_height_m / 5.0;
    const double d     = cfg.cm_pelvis_ratio * L;
    const double theta = rcfg.theta_max_deg * kDegToRad
                       * std::tanh(cm.velocity.x / rcfg.v_ref);
    return { cm.position.x - d * std::sin(theta),
             cm.position.y - d * std::cos(theta) };
}

// Clamp pos to within radius r of center.
Vec2 clampToCircle(Vec2 pos, Vec2 center, double r)
{
    const double dx   = pos.x - center.x;
    const double dy   = pos.y - center.y;
    const double dist = std::sqrt(dx * dx + dy * dy);
    if (dist > r && dist > kEpsLength)
        return { center.x + dx * (r / dist),
                 center.y + dy * (r / dist) };
    return pos;
}

struct HeightTargetState {
    double h_ip = 0.0;
    double speed_drop = 0.0;
    double slope_drop = 0.0;
    double y_tgt = 0.0;
};

struct LandingFootPlan {
    Vec2 left  = {0.0, 0.0};
    Vec2 right = {0.0, 0.0};
};

double smooth01(double x)
{
    x = std::clamp(x, 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

LandingFootPlan planLandingFeet(const CharacterState& ch,
                                const StandingConfig& stand_cfg,
                                const Terrain&        terrain,
                                const Vec2&           pelvis,
                                double                reach_radius,
                                double                L)
{
    LandingFootPlan plan;
    const double half = stand_cfg.d_pref * 0.5 * L;
    const double lx = clampTerrainEndpointX(terrain, pelvis, reach_radius,
                                            pelvis.x, pelvis.x - ch.facing * half);
    const double rx = clampTerrainEndpointX(terrain, pelvis, reach_radius,
                                            pelvis.x, pelvis.x + ch.facing * half);
    plan.left  = { lx, terrain.height_at(lx) };
    plan.right = { rx, terrain.height_at(rx) };
    return plan;
}

double predictLandingTime(const CMState&                           cm,
                          const CharacterConfig&                   char_cfg,
                          const CharacterReconstructionConfig&      recon_cfg,
                          const Terrain&                           terrain,
                          double                                   g,
                          double                                   L)
{
    const double d = char_cfg.cm_pelvis_ratio * L;
    const double theta = recon_cfg.theta_max_deg * kDegToRad
                       * std::tanh(cm.velocity.x / recon_cfg.v_ref);
    const double pelvis_drop = d * std::cos(theta);

    constexpr double dt_probe = 1.0 / 120.0;
    constexpr int max_steps = 240; // 2 seconds
    for (int i = 1; i <= max_steps; ++i) {
        const double t = dt_probe * static_cast<double>(i);
        const double x = cm.position.x + cm.velocity.x * t;
        const double y = cm.position.y + cm.velocity.y * t - 0.5 * g * t * t;
        const double pelvis_y = y - pelvis_drop;
        const double threshold = terrain.height_at(x) + 2.0 * L;
        if (pelvis_y <= threshold)
            return t;
    }
    return dt_probe * static_cast<double>(max_steps);
}

void beginJumpPreload(CharacterState&   ch,
                      bool              run_mode,
                      double            speed_abs,
                      double            L,
                      const CMState&    cm,
                      const JumpConfig& jump_cfg)
{
    ch.jump_preload_active = true;
    ch.jump_flight_active  = false;
    ch.jump_targets_valid  = false;
    ch.jump_preload_t      = 0.0;
    ch.jump_origin_mode    = run_mode ? LocomotionState::Running
                           : (speed_abs > 0.2 ? LocomotionState::Walking
                                              : LocomotionState::Standing);
    switch (ch.jump_origin_mode) {
        case LocomotionState::Running:
            ch.jump_preload_duration = jump_cfg.preload_dur_run;
            ch.jump_preload_depth    = jump_cfg.preload_depth_run * L;
            break;
        case LocomotionState::Walking:
            ch.jump_preload_duration = jump_cfg.preload_dur_walk;
            ch.jump_preload_depth    = jump_cfg.preload_depth_walk * L;
            break;
        case LocomotionState::Standing:
        default:
            ch.jump_preload_duration = jump_cfg.preload_dur_stand;
            ch.jump_preload_depth    = jump_cfg.preload_depth_stand * L;
            break;
    }
    ch.jump_takeoff_cm_vel = cm.velocity;
}

void beginAirborneLandingProtocol(CharacterState&                       ch,
                                  const AppConfig&                      config,
                                  const Terrain&                        terrain,
                                  const CharacterReconstructionConfig&  recon_cfg,
                                  const CMState&                        cm,
                                  double                                g,
                                  double                                L)
{
    ch.jump_flight_active  = true;
    ch.jump_takeoff_cm_pos = cm.position;
    ch.jump_takeoff_cm_vel = cm.velocity;
    ch.jump_left_start     = ch.foot_left.pos;
    ch.jump_right_start    = ch.foot_right.pos;
    ch.jump_tuck_height    = config.jump.tuck_height_ratio * L;
    ch.jump_total_flight_time = predictLandingTime(cm, config.character, recon_cfg,
                                                   terrain, g, L);
    ch.jump_time_remaining = ch.jump_total_flight_time;

    const double t_land = ch.jump_total_flight_time;
    const CMState predicted_cm{
        { cm.position.x + cm.velocity.x * t_land,
          cm.position.y + cm.velocity.y * t_land - 0.5 * g * t_land * t_land },
        cm.velocity,
        {0.0, 0.0}
    };
    const Vec2 predicted_pelvis = reconstructPelvis(predicted_cm, config.character, recon_cfg);
    const LandingFootPlan plan = planLandingFeet(ch, config.standing, terrain,
                                                 predicted_pelvis, 2.0 * L, L);
    ch.jump_left_target = plan.left;
    ch.jump_right_target = plan.right;
    ch.jump_targets_valid = true;

    auto prepFoot = [](FootState& foot) {
        foot.pinned = false;
        foot.on_ground = false;
        foot.airborne = true;
        foot.swinging = false;
    };
    prepFoot(ch.foot_left);
    prepFoot(ch.foot_right);
}

void updateJumpLandingTargets(CharacterState&                      ch,
                              const AppConfig&                     config,
                              const Terrain&                       terrain,
                              const CharacterReconstructionConfig& recon_cfg,
                              const CMState&                       cm,
                              double                               g,
                              double                               L)
{
    const double t_land = predictLandingTime(cm, config.character, recon_cfg, terrain, g, L);
    ch.jump_total_flight_time = std::max(ch.jump_total_flight_time, t_land);
    ch.jump_time_remaining = t_land;

    const CMState predicted_cm{
        { cm.position.x + cm.velocity.x * t_land,
          cm.position.y + cm.velocity.y * t_land - 0.5 * g * t_land * t_land },
        cm.velocity,
        {0.0, 0.0}
    };
    const Vec2 predicted_pelvis = reconstructPelvis(predicted_cm, config.character, recon_cfg);
    const LandingFootPlan plan = planLandingFeet(ch, config.standing, terrain,
                                                 predicted_pelvis, 2.0 * L, L);
    const double target_alpha = smooth01(1.0 - t_land / std::max(ch.jump_total_flight_time, 1.0e-4));
    ch.jump_left_target  = ch.jump_left_target  + (plan.left  - ch.jump_left_target)  * std::max(0.15, target_alpha);
    ch.jump_right_target = ch.jump_right_target + (plan.right - ch.jump_right_target) * std::max(0.15, target_alpha);
    ch.jump_targets_valid = true;
}

void updateJumpFeetInFlight(CharacterState& ch,
                            const Vec2&     pelvis,
                            double          progress)
{
    const double s = smooth01(progress);
    const double tuck_weight = 4.0 * s * (1.0 - s);
    const double tuck = ch.jump_tuck_height * tuck_weight;
    auto updateFoot = [&](FootState& foot, Vec2 start, Vec2 target) {
        const double base_x = start.x + (target.x - start.x) * s;
        const double tucked_x = pelvis.x + 0.18 * (target.x - pelvis.x);
        foot.pos.x = base_x + (tucked_x - base_x) * (0.70 * tuck_weight);
        foot.pos.y = start.y + (target.y - start.y) * s + tuck;
        foot.on_ground = false;
        foot.airborne = true;
        foot.pinned = false;
        foot.swinging = false;
    };
    updateFoot(ch.foot_left,  ch.jump_left_start,  ch.jump_left_target);
    updateFoot(ch.foot_right, ch.jump_right_start, ch.jump_right_target);
}

RunTimingTargets computeRunTimingTargets(const RunConfig& run_cfg,
                                         double           speed_abs,
                                         double           max_speed,
                                         double           L)
{
    RunTimingTargets out;
    const double usable_max = std::max(max_speed, 1.0e-4);
    out.speed_ratio = std::clamp(speed_abs / usable_max, 0.0, 1.0);

    out.cadence_spm = std::lerp(run_cfg.cadence_spm_min, run_cfg.cadence_spm_max, out.speed_ratio);

    const double stride_L_min = run_cfg.stride_len_min;
    const double stride_L_max = std::max(run_cfg.stride_len, stride_L_min + 0.2);
    const double stride_len_m = std::lerp(stride_L_min, stride_L_max, out.speed_ratio) * L;

    if (speed_abs > 1.0e-4) {
        out.step_period = stride_len_m / (2.0 * speed_abs);
    } else {
        out.step_period = 60.0 / run_cfg.cadence_spm_min;
    }

    const double preferred_step_period = 60.0 / std::max(out.cadence_spm, 1.0e-4);
    out.step_period = 0.5 * (out.step_period + preferred_step_period);
    out.step_period = std::clamp(out.step_period, 0.28, 0.42);

    out.contact_time = std::lerp(0.26, 0.22, out.speed_ratio);
    out.contact_time = std::min(out.contact_time, out.step_period - 0.03);
    out.contact_time = std::max(out.contact_time, 0.16);
    out.flight_time = std::max(0.0, out.step_period - out.contact_time);
    out.step_length = speed_abs * out.step_period;
    out.touchdown_ahead = std::lerp(0.25 * L,
                                    std::max(0.30 * L, run_cfg.stability_margin * L),
                                    out.speed_ratio);
    return out;
}

double computeRunLandingX(const RunConfig&       run_cfg,
                          const CharacterState&  ch,
                          const FootState&       stance_foot,
                          const Vec2&            pelvis,
                          double                 velocity_x,
                          double                 reach_radius,
                          double                 L,
                          const RunTimingTargets& timing,
                          double                 ref_slope)
{
    const double facing = ch.facing;
    const double future_pelvis_x = pelvis.x + velocity_x * 0.06;
    double front_extent = std::lerp(0.45 * L, 0.80 * L, timing.speed_ratio);
    const double uphill_alignment = facing * ref_slope;
    if (uphill_alignment > 0.0) {
        const double uphill_strength = std::clamp(uphill_alignment / 0.35, 0.0, 1.0);
        front_extent += std::lerp(0.0, 0.30 * L, uphill_strength);
    }
    const double raw_tx = future_pelvis_x + facing * front_extent;

    const double max_step = run_cfg.max_step_L * L;
    const double support_eps = 0.08 * L;
    const double reach_lo = pelvis.x - 0.95 * reach_radius;
    const double reach_hi = pelvis.x + 0.95 * reach_radius;
    const double support_lo = std::min(stance_foot.pos.x + facing * support_eps,
                                       stance_foot.pos.x + facing * max_step);
    const double support_hi = std::max(stance_foot.pos.x + facing * support_eps,
                                       stance_foot.pos.x + facing * max_step);
    const double lo = std::max(reach_lo, support_lo);
    const double hi = std::min(reach_hi, support_hi);

    if (lo <= hi)
        return std::clamp(raw_tx, lo, hi);

    return std::clamp(raw_tx, support_lo, support_hi);
}

double computeStepLandingX(const WalkConfig&   walk_cfg,
                           const CharacterState& ch,
                           const FootState&    stance_foot,
                           const Vec2&         pelvis,
                           double              xi,
                           double              L,
                           double              reach_radius,
                           double              ref_slope)
{
    const double downhill_trim = 0.5 * walk_cfg.downhill_step_bonus
                               * ch.downhill_crouch;
    const double margin    = std::max(0.0, walk_cfg.stability_margin - downhill_trim);
    const double raw_tx    = xi + ch.facing * margin * L;

    // On a slope the foot must travel both horizontally and vertically.
    // Maximum horizontal reach = physical_reach / sqrt(1 + slope²).
    // This prevents the planner from requesting a landing that the IK cannot reach.
    const double slope_factor = std::sqrt(1.0 + ref_slope * ref_slope);
    const double max_reach    = 0.95 * reach_radius / slope_factor;
    const double max_step  = walk_cfg.max_step_L * L;
    const double base_lo   = std::max(pelvis.x - max_reach, stance_foot.pos.x - max_step);
    const double base_hi   = std::min(pelvis.x + max_reach, stance_foot.pos.x + max_step);

    double lo = base_lo;
    double hi = base_hi;

    // The landing foot must create a support interval that contains xi with
    // a small margin. This prevents swapping the foot order while leaving
    // the body outside the new support.
    const double support_margin = 0.05 * L;
    if (stance_foot.pos.x <= xi)
        lo = std::max(lo, xi + support_margin);
    else
        hi = std::min(hi, xi - support_margin);

    if (lo <= hi)
        return std::clamp(raw_tx, lo, hi);

    // If the geometry cannot satisfy the support condition, fall back
    // to the reach/step limits instead of refusing the step outright.
    return std::clamp(raw_tx, base_lo, base_hi);
}

void refreshSwingArcProfile(FootState&         foot,
                            const Terrain&     terrain,
                            double             L,
                            const StepConfig&  step_cfg,
                            const WalkConfig&  walk_cfg,
                            double             speed_abs,
                            double             walk_max_speed)
{
    const double tx_local = foot.swing_target.x;

    // A step with large vertical drop/rise covers more total arc distance than
    // its horizontal extent alone. Scale down the parameter advance rate so the
    // foot takes proportionally longer — downhill steps slow the swing naturally.
    {
        const double dx      = tx_local - foot.swing_start.x;
        const double dy      = foot.swing_target.y - foot.swing_start.y;
        const double hdist   = std::abs(dx);
        const double arc_len = std::sqrt(dx * dx + dy * dy);
        foot.swing_speed_scale = (arc_len > 1.0e-9)
            ? std::max(0.30, hdist / arc_len)
            : 1.0;
    }

    // Base lift from StepConfig. Uphill adds lift (foot must clear rising terrain);
    // downhill reduces it (terrain drops away, less clearance needed).
    // Speed adds a small bonus for larger, faster arcs.
    {
        const double dx         = tx_local - foot.swing_start.x;
        const double dy         = foot.swing_target.y - foot.swing_start.y;
        const double hdist      = std::abs(dx);
        const double step_slope = (hdist > 1.0e-9) ? (dy / hdist) : 0.0;
        const double h_base     = step_cfg.h_clear_ratio * L;
        const double h_slope    = walk_cfg.h_clear_slope_factor * L * step_slope;
        const double h_speed    = walk_cfg.h_clear_speed_factor * L
                                * std::clamp(speed_abs / std::max(walk_max_speed, 1.0e-4),
                                             0.0, 1.0);

        double max_terrain_bulge = 0.0;
        for (int s = 1; s <= 3; ++s) {
            const double ts       = s * 0.25;
            const double x_samp   = foot.swing_start.x + ts * dx;
            const double y_linear = foot.swing_start.y + ts * dy;
            const double bulge    = terrain.height_at(x_samp) - y_linear;
            max_terrain_bulge     = std::max(max_terrain_bulge, bulge);
        }

        const double h_terrain = max_terrain_bulge + walk_cfg.h_clear_min_ratio * L;
        foot.swing_h_clear = std::max(h_terrain,
                                      std::max(walk_cfg.h_clear_min_ratio * L,
                                               h_base + h_slope + h_speed));
    }
}

double estimateSwingRemainingTime(const FootState& foot,
                                  const WalkConfig& walk_cfg)
{
    const double rate = std::max(1.0e-4, walk_cfg.step_speed * foot.swing_speed_scale);
    return std::max(0.0, 1.0 - foot.swing_t) / rate;
}

double retargetLandingRecoveryX(double                 target_x,
                                const CharacterState&  ch,
                                const FootState&       stance_foot,
                                const Vec2&            pelvis,
                                double                 future_cm_x,
                                double                 reach_radius,
                                double                 L,
                                double                 ref_slope,
                                const WalkConfig&      walk_cfg,
                                double                 recovery_gain)
{
    const double normalized_gain = std::clamp(recovery_gain / 1.8, 0.0, 1.0);
    const double valid_back      = std::lerp(0.34 * L, 0.16 * L, normalized_gain);
    const double extra_forward   = std::lerp(0.04 * L, 0.12 * L, normalized_gain);
    const double behind_future   = (future_cm_x - target_x) * ch.facing;
    if (behind_future <= valid_back)
        return target_x;

    const double desired_x = future_cm_x - ch.facing * valid_back
                           + ch.facing * extra_forward;
    const double slope_factor = std::sqrt(1.0 + ref_slope * ref_slope);
    const double max_reach    = 0.95 * reach_radius / slope_factor;
    const double max_step     = walk_cfg.max_step_L * L;
    const double base_lo      = std::max(pelvis.x - max_reach, stance_foot.pos.x - max_step);
    const double base_hi      = std::min(pelvis.x + max_reach, stance_foot.pos.x + max_step);

    return std::clamp(desired_x, base_lo, base_hi);
}

void beginSwingStep(FootState&       step_foot,
                    FootState&       stance_foot,
                    CharacterState&  ch,
                    double           tx,
                    const Terrain&   terrain,
                    bool             corrective_followthrough,
                    double           L,
                    const StepConfig&  step_cfg,
                    const WalkConfig&  walk_cfg,
                    double           speed_abs,
                    double           walk_max_speed)
{
    step_foot.pinned       = false;
    step_foot.swinging     = true;
    step_foot.on_ground    = false;   // foot is leaving ground — clear immediately so the
                                      // height formula stops using this foot for ipY min()
    step_foot.swing_start  = step_foot.pos;
    step_foot.swing_target = { tx, terrain.height_at(tx) };
    step_foot.swing_t      = 0.0;
    refreshSwingArcProfile(step_foot, terrain, L, step_cfg, walk_cfg, speed_abs, walk_max_speed);

    if (corrective_followthrough) {
        const double step_dx = tx - step_foot.pos.x;
        ch.recovery_followthrough_active = true;
        ch.recovery_followthrough_dir    = (std::abs(step_dx) > 1.0e-6)
                                         ? std::copysign(1.0, step_dx)
                                         : ch.facing;
    } else {
        ch.recovery_followthrough_active = false;
        ch.recovery_followthrough_dir    = 0.0;
    }

    if (!stance_foot.pinned && !stance_foot.swinging) {
        stance_foot.pinned        = true;
        stance_foot.pinned_pos    = stance_foot.pos;
        stance_foot.pinned_normal = stance_foot.ground_normal;
    }
    ch.step_cooldown = 0.05;
}

void releaseFeetAirborne(CharacterState& ch)
{
    auto releaseFootAirborne = [](FootState& foot) {
        foot.swinging  = false;
        foot.pinned    = false;
        foot.on_ground = false;
        foot.airborne  = false;
    };
    releaseFootAirborne(ch.foot_left);
    releaseFootAirborne(ch.foot_right);
}

void bootstrapFeetOnLanding(CharacterState&       ch,
                            const StandingConfig& stand_cfg,
                            const Terrain&        terrain,
                            const Vec2&           pelvis,
                            double                reach_radius,
                            double                L)
{
    const double half = stand_cfg.d_pref * 0.5 * L;
    const double lx = clampTerrainEndpointX(terrain, pelvis, reach_radius,
                                            pelvis.x, pelvis.x - ch.facing * half);
    const double rx = clampTerrainEndpointX(terrain, pelvis, reach_radius,
                                            pelvis.x, pelvis.x + ch.facing * half);

    ch.foot_left.pos        = { lx, terrain.height_at(lx) };
    ch.foot_left.pinned     = true;
    ch.foot_left.pinned_pos = ch.foot_left.pos;
    ch.foot_left.swinging   = false;
    ch.foot_left.airborne   = false;
    ch.foot_left.on_ground  = true;

    ch.foot_right.pos        = { rx, terrain.height_at(rx) };
    ch.foot_right.pinned     = true;
    ch.foot_right.pinned_pos = ch.foot_right.pos;
    ch.foot_right.swinging   = false;
    ch.foot_right.airborne   = false;
    ch.foot_right.on_ground  = true;
}

void initializeFeetUnderPelvis(CharacterState& ch,
                               const Terrain&  terrain,
                               const Vec2&     pelvis,
                               double          L,
                               double          d_pref)
{
    const double half = d_pref * 0.5 * L;
    ch.foot_left.pos  = { pelvis.x - half, terrain.height_at(pelvis.x - half) };
    ch.foot_right.pos = { pelvis.x + half, terrain.height_at(pelvis.x + half) };
    ch.feet_initialized = true;
}

void applyPinnedFootPositions(CharacterState& ch)
{
    if (ch.foot_left.pinned)  ch.foot_left.pos  = ch.foot_left.pinned_pos;
    if (ch.foot_right.pinned) ch.foot_right.pos = ch.foot_right.pinned_pos;
}

void applyDraggedFootPositions(CharacterState&   ch,
                               const InputFrame& input)
{
    if (input.foot_left_drag) {
        ch.foot_left.pos = input.foot_left_pos;
        if (ch.foot_left.pinned) ch.foot_left.pinned_pos = input.foot_left_pos;
    }
    if (input.foot_right_drag) {
        ch.foot_right.pos = input.foot_right_pos;
        if (ch.foot_right.pinned) ch.foot_right.pinned_pos = input.foot_right_pos;
    }
}

void advanceSwingFoot(FootState&        foot,
                      const Terrain&    terrain,
                      const Vec2&       pelvis,
                      double            dt,
                      double            reach_radius,
                      const WalkConfig& walk_cfg)
{
    if (!foot.swinging) return;

    // swing_speed_scale < 1 on steep steps (more vertical travel → slower param advance).
    foot.swing_t += walk_cfg.step_speed * foot.swing_speed_scale * dt;
    if (foot.swing_t >= 1.0) {
        // Land: snap to terrain, pin.
        // Clamp landing x to current pelvis reach in case pelvis moved during swing.
        const double land_raw = foot.swing_target.x;
        const double land_x   = std::clamp(land_raw,
                                           pelvis.x - reach_radius,
                                           pelvis.x + reach_radius);
        foot.swing_t       = 1.0;
        foot.swinging      = false;
        foot.pos           = { land_x, terrain.height_at(land_x) };
        foot.pinned        = true;
        foot.pinned_pos    = foot.pos;
        foot.pinned_normal = terrain.normal_at(land_x);
        return;
    }

    const double t  = foot.swing_t;
    const double sx = foot.swing_start.x;
    const double tx = foot.swing_target.x;
    const double sy = foot.swing_start.y;
    const double ty = terrain.height_at(tx);
    // Horizontal: linear interpolation
    const double px = sx + t * (tx - sx);
    // Vertical: linear blend + parabolic lift (h_clear stored per-foot, set at step initiation)
    const double py = sy + t * (ty - sy) + foot.swing_h_clear * 4.0 * t * (1.0 - t);
    // Early landing: if arc dips below terrain, land immediately
    const double terrain_y = terrain.height_at(px);
    if (py < terrain_y) {
        foot.swing_t       = 1.0;
        foot.swinging      = false;
        foot.pos           = { px, terrain_y };
        foot.pinned        = true;
        foot.pinned_pos    = foot.pos;
        foot.pinned_normal = terrain.normal_at(px);
        return;
    }

    foot.pos = { px, py };
}

void applyFootConstraints(CharacterState& ch,
                          const Terrain&  terrain,
                          const Vec2&     pelvis,
                          double          reach_radius)
{
    // Constraint 1: terrain — non-swinging feet only.
    if (!ch.foot_left.swinging && !ch.foot_left.airborne)  applyGroundConstraint(ch.foot_left,  terrain);
    if (!ch.foot_right.swinging && !ch.foot_right.airborne) applyGroundConstraint(ch.foot_right, terrain);

    // Constraint 2: 2L reach circle — global, always enforced.
    ch.foot_left.pos  = clampToCircle(ch.foot_left.pos,  pelvis, reach_radius);
    ch.foot_right.pos = clampToCircle(ch.foot_right.pos, pelvis, reach_radius);

    if (!ch.foot_left.swinging && !ch.foot_left.airborne)  applyGroundConstraint(ch.foot_left,  terrain);
    if (!ch.foot_right.swinging && !ch.foot_right.airborne) applyGroundConstraint(ch.foot_right, terrain);
}

struct StepTriggerEval {
    double front_x = 0.0;
    double rear_x = 0.0;
    double d_rear = 0.0;
    bool   step_left_xcom = false;
    bool   xcom_trigger = false;
    bool   rear_trigger = false;
};

StepTriggerEval evaluateStepTriggers(const CharacterState& ch,
                                     double                eff_lx,
                                     double                eff_rx,
                                     double                xi,
                                     double                velocity_x,
                                     double                eps_step,
                                     const Vec2&           pelvis,
                                     double                L,
                                     double                d_rear_max)
{
    StepTriggerEval eval;
    const double facing = ch.facing;
    eval.front_x = (facing >= 0.0) ? std::max(eff_lx, eff_rx)
                                   : std::min(eff_lx, eff_rx);
    eval.rear_x = (facing >= 0.0) ? std::min(eff_lx, eff_rx)
                                  : std::max(eff_lx, eff_rx);
    eval.step_left_xcom = (facing >= 0.0) ? (eff_lx < eff_rx)
                                          : (eff_lx > eff_rx);
    eval.xcom_trigger = std::abs(velocity_x) > eps_step
                     && (xi - eval.front_x) * facing > 0.0;
    eval.d_rear = (pelvis.x - eval.rear_x) * facing;
    eval.rear_trigger = eval.d_rear > d_rear_max * L;
    return eval;
}

template <typename LaunchStepFn>
void tryRecoveryStepOnLiftOff(const CharacterState& ch,
                              bool                  airborne_final,
                              bool                  any_swinging,
                              bool                  was_grounded_L,
                              bool                  was_grounded_R,
                              double                input_dir,
                              LaunchStepFn&&        launchStep)
{
    const bool lift_off_L = was_grounded_L && !ch.foot_left.on_ground && !ch.foot_left.swinging;
    const bool lift_off_R = was_grounded_R && !ch.foot_right.on_ground && !ch.foot_right.swinging;
    if (airborne_final || any_swinging) return;

    if (lift_off_L && ch.foot_right.on_ground) {
        launchStep(true, std::abs(input_dir) <= kInputDirDeadzone);
        SIM_LOG("Step trigger: LEFT  lift_off=1\n");
    } else if (lift_off_R && ch.foot_left.on_ground) {
        launchStep(false, std::abs(input_dir) <= kInputDirDeadzone);
        SIM_LOG("Step trigger: RIGHT  lift_off=1\n");
    }
}

void unpinLiftedFeet(CharacterState& ch,
                     bool            airborne_final,
                     bool            was_grounded_L,
                     bool            was_grounded_R)
{
    if (airborne_final) return;

    auto unpinIfLifted = [](FootState& foot, bool was_grounded) {
        if (foot.pinned && was_grounded && !foot.on_ground && !foot.swinging)
            foot.pinned = false;
    };
    unpinIfLifted(ch.foot_left,  was_grounded_L);
    unpinIfLifted(ch.foot_right, was_grounded_R);
}

void cacheXCoMState(SimState&              state,
                    const CharacterState&  ch,
                    const CMState&         cm,
                    const WalkConfig&      walk_cfg,
                    double                 eff_lx,
                    double                 eff_rx,
                    double                 xi,
                    double                 L)
{
    const double front_x = ch.feet_initialized
        ? ((ch.facing >= 0.0) ? std::max(eff_lx, eff_rx) : std::min(eff_lx, eff_rx))
        : cm.position.x;
    state.xi          = xi;
    state.xi_target_x = xi + ch.facing * walk_cfg.stability_margin * L;
    state.xi_trigger  = ch.feet_initialized
        && std::abs(cm.velocity.x) > walk_cfg.eps_step
        && (xi - front_x) * ch.facing > 0.0;
}

void updateDownhillCrouch(CharacterState&      ch,
                          const WalkConfig&    walk_cfg,
                          const PhysicsConfig& physics_cfg,
                          double               ref_slope,
                          double               velocity_x,
                          double               max_spd,
                          double               dt)
{
    const double slope_motion = ref_slope * std::tanh(
        velocity_x / std::max(physics_cfg.hold_speed, 1.0e-4));
    const double downhill_ratio = std::clamp(std::max(0.0, -slope_motion) / 0.5, 0.0, 1.0);
    const double speed_ratio = std::clamp(std::abs(velocity_x) / max_spd, 0.0, 1.0);
    const double crouch_target = downhill_ratio * speed_ratio;
    const double crouch_tau = (crouch_target > ch.downhill_crouch)
        ? std::max(walk_cfg.downhill_crouch_tau, 1.0e-4)
        : std::max(walk_cfg.downhill_relax_tau,  1.0e-4);
    const double crouch_alpha = 1.0 - std::exp(-dt / crouch_tau);
    ch.downhill_crouch += (crouch_target - ch.downhill_crouch) * crouch_alpha;
    ch.downhill_crouch = std::clamp(ch.downhill_crouch, 0.0, 1.0);
}

bool isSlideActive(const FootState& foot, double velocity_x)
{
    if (!foot.on_ground || foot.swinging) return false;

    const double slope_strength = std::abs(foot.ground_normal.x);
    const double speed = std::abs(velocity_x);
    return slope_strength >= 0.14 && speed >= 0.55;
}

HeightTargetState computeHeightTargetState(const CharacterState& ch,
                                           const CMState&        cm,
                                           const WalkConfig&     walk_cfg,
                                           const PhysicsConfig&  physics_cfg,
                                           double                ref_ground,
                                           double                ref_slope,
                                           double                h_nominal,
                                           double                cm_pelvis_ratio,
                                           double                L)
{
    HeightTargetState state;

    // Inverted pendulum arc radius.
    //
    // At mid-stance (CM directly over stance foot, dx = 0):
    //   y_cm   = y_foot + R_bob
    //   y_pelvis = y_cm − cm_pelvis_ratio·L  →  pelvis-to-foot = (2 − leg_flex_coeff)·L
    //
    // R_bob = (2 − leg_flex_coeff + cm_pelvis_ratio) · L
    //   leg_flex_coeff = 0  → fully extended leg (2L pelvis-to-foot)
    //   leg_flex_coeff = 0.1 → 10% knee bend (1.9L)
    //
    // bob_scale amplifies the descent away from the peak (values >1 = more expressive).
    // bob_amp caps the maximum drop so a distant foot doesn't crash the CM target.
    const double R_bob   = (2.0 - walk_cfg.leg_flex_coeff + cm_pelvis_ratio) * L;
    const double bob_max = walk_cfg.bob_amp * L;

    auto ipY = [&](const FootState& foot) -> double {
        const double dx    = cm.position.x - foot.pos.x;
        const double h_arc = std::sqrt(std::max(0.0, R_bob * R_bob - dx * dx));
        const double dev   = std::max(h_arc - R_bob, -bob_max);  // ≤ 0, capped
        return foot.pos.y + R_bob + walk_cfg.bob_scale * dev;
    };

    // Combine grounded feet with min(): gives the valley at double support.
    //
    // Single support: only one foot contributes → its arc rises to peak at
    //   mid-stance then falls.
    // Double support: min picks the lower arc (trailing foot, already past
    //   mid-stance) → natural valley at the heel-strike / toe-off transition.
    // Airborne: fall back to smoothed terrain reference.
    const bool grounded_L = ch.foot_left.on_ground;
    const bool grounded_R = ch.foot_right.on_ground;

    double y_ip;
    if (grounded_L && grounded_R) {
        y_ip = std::min(ipY(ch.foot_left), ipY(ch.foot_right));
    } else if (grounded_L) {
        y_ip = ipY(ch.foot_left);
    } else if (grounded_R) {
        y_ip = ipY(ch.foot_right);
    } else {
        y_ip = ref_ground + h_nominal;
    }

    state.h_ip = y_ip - ref_ground;  // keep debug field: effective height above terrain ref

    state.speed_drop = walk_cfg.max_speed_drop * L
                     * std::clamp(std::abs(cm.velocity.x)
                                  / physics_cfg.walk_max_speed, 0.0, 1.0);
    state.slope_drop = walk_cfg.max_slope_drop * L
                     * std::clamp(std::abs(ref_slope) / 0.5, 0.0, 1.0);
    const double downhill_crouch_drop = walk_cfg.downhill_crouch_max
                                      * L * ch.downhill_crouch;

    state.y_tgt = y_ip + walk_cfg.cm_height_offset
                - state.speed_drop - state.slope_drop - downhill_crouch_drop;
    return state;
}

double computeHorizontalAcceleration(const CharacterState& ch,
                                     const CMState&        cm,
                                     const PhysicsConfig&  physics_cfg,
                                     double                input_dir,
                                     double                sin_t,
                                     double                cos_t,
                                     double                g,
                                     bool                  airborne_pre)
{
    double accel_x = 0.0;

    if (!airborne_pre && std::abs(input_dir) > kInputDirDeadzone) {
        accel_x += input_dir * physics_cfg.accel * cos_t;
    } else if (!airborne_pre) {
        double friction = physics_cfg.floor_friction;
        if (ch.recovery_followthrough_active
            && ch.recovery_followthrough_dir * cm.velocity.x > 0.0) {
            const FootState* swing_foot = ch.foot_left.swinging ? &ch.foot_left
                                        : ch.foot_right.swinging ? &ch.foot_right
                                                                 : nullptr;
            const double t = swing_foot ? std::clamp(swing_foot->swing_t, 0.0, 1.0) : 1.0;
            const double friction_scale = 0.25 + 0.75 * t;
            friction *= friction_scale;
        }
        accel_x += -friction * cm.velocity.x;
    }
    if (!airborne_pre
        && (std::abs(input_dir) > kInputDirDeadzone
            || std::abs(cm.velocity.x) > physics_cfg.hold_speed)) {
        accel_x += -g * sin_t * cos_t;
    }

    return accel_x;
}

void integrateHorizontalMotion(CMState&             cm,
                               const CharacterState& ch,
                               const PhysicsConfig& physics_cfg,
                               double               input_dir,
                               double               sin_t,
                               double               cos_t,
                               double               g,
                               bool                 airborne_pre,
                               double               max_spd,
                               double               dt,
                               Vec2&                accel)
{
    accel.x = computeHorizontalAcceleration(ch, cm, physics_cfg, input_dir,
                                            sin_t, cos_t, g, airborne_pre);
    cm.velocity.x += accel.x * dt;

    if (std::abs(input_dir) > kInputDirDeadzone
        && cm.velocity.x * input_dir > 0.0
        && std::abs(cm.velocity.x) > max_spd) {
        cm.velocity.x = std::copysign(max_spd, input_dir);
    } else if (std::abs(input_dir) <= kInputDirDeadzone
               && std::abs(cm.velocity.x) < physics_cfg.stop_speed) {
        cm.velocity.x = 0.0;
    }

    cm.position.x += cm.velocity.x * dt;
}

bool integrateVerticalMotion(CMState&                             cm,
                             Vec2&                                accel,
                             const Terrain&                       terrain,
                             const CharacterConfig&               char_cfg,
                             const CharacterReconstructionConfig&  recon_cfg,
                             const PhysicsConfig&                 physics_cfg,
                             double                               y_tgt,
                             double                               ref_ground,
                             double                               L,
                             double                               g,
                             double                               dt)
{
    const Vec2 pelvis_post = reconstructPelvis(cm, char_cfg, recon_cfg);
    const bool airborne    = (pelvis_post.y - terrain.height_at(pelvis_post.x) > 2.0 * L);

    if (physics_cfg.gravity_enabled)
        accel.y -= g;

    if (physics_cfg.spring_enabled && !airborne) {
        const double delta  = y_tgt - cm.position.y;
        const double band   = std::max(0.25 * physics_cfg.d_soft, 1.0e-4);
        const double t      = std::clamp(delta / band, 0.0, 1.0);
        const double act    = t * t * (3.0 - 2.0 * t);
        if (act > 0.0) {
            const double vy_w = physics_cfg.vy_max
                              * std::tanh(std::max(0.0, delta) / physics_cfg.d_soft);
            accel.y += act * std::max(0.0, (vy_w - cm.velocity.y) * physics_cfg.vy_tau);
        }
    }

    cm.acceleration = accel;
    cm.velocity.y  += accel.y * dt;
    cm.position.y  += cm.velocity.y * dt;

    // Skipped when airborne: ref_ground is meaningless far above terrain and
    // would freeze the CM at the launch height as the character descends.
    if (!airborne) {
        const double min_cm_y   = ref_ground + L + char_cfg.cm_pelvis_ratio * L;
        const double guard_band = std::max(0.25 * L, 0.5 * physics_cfg.d_soft);
        if (cm.position.y - min_cm_y < guard_band) {
            const double pen = std::max(0.0, min_cm_y - cm.position.y);
            if (pen > 0.0) {
                cm.position.y = std::min(min_cm_y,
                    cm.position.y + physics_cfg.vy_max * dt
                                  * std::tanh(pen / guard_band));
            }
            if (cm.velocity.y < 0.0) {
                cm.velocity.y += (-cm.velocity.y)
                               * std::clamp(physics_cfg.vy_tau * dt, 0.0, 1.0);
            }
        }
        if (cm.position.y < min_cm_y) {
            cm.position.y = min_cm_y;
            if (cm.velocity.y < 0.0) cm.velocity.y = 0.0;
        }
    }

    return airborne;
}

// Collision constraint: push foot out of terrain if penetrating.
// on_ground = true iff foot center is on or very near the terrain surface.
// A small tolerance avoids false lift-off flicker after reach clamping on slopes.
void refreshGroundContact(FootState& foot, const Terrain& terrain)
{
    const double ty    = terrain.height_at(foot.pos.x);
    const Vec2   n     = terrain.normal_at(foot.pos.x);   // outward (upward-ish) unit normal

    // Signed distance along normal from surface to foot (positive = above surface)
    const double gap_n = (foot.pos.y - ty) * n.y;

    foot.on_ground     = (gap_n <= kGroundContactSnapEps);
    foot.ground_normal = foot.on_ground ? n : Vec2{0.0, 1.0};
}

void applyGroundConstraint(FootState& foot, const Terrain& terrain)
{
    const double ty    = terrain.height_at(foot.pos.x);
    const Vec2   n     = terrain.normal_at(foot.pos.x);   // outward (upward-ish) unit normal

    // Signed distance along normal from surface to foot (positive = above surface)
    const double gap_n = (foot.pos.y - ty) * n.y;

    if (gap_n < 0.0) {
        // Actual penetration — resolve by pushing exactly onto surface
        foot.pos.x -= gap_n * n.x;
        foot.pos.y -= gap_n * n.y;
    } else if (gap_n <= kGroundContactSnapEps) {
        // Tiny positive gaps are usually numerical drift after reach clamping.
        // Snap back to the surface so contact does not flicker on slopes.
        foot.pos.x -= gap_n * n.x;
        foot.pos.y -= gap_n * n.y;
    }

    refreshGroundContact(foot, terrain);
}

void blendWalkRunConfig(WalkConfig&      eff_walk,
                        StepConfig&      eff_step,
                        PhysicsConfig&   eff_physics,
                        const AppConfig& cfg,
                        double           rb)
{
    eff_walk.step_speed          = std::lerp(cfg.walk.step_speed,         cfg.run.step_speed,        rb);
    eff_walk.stability_margin    = std::lerp(cfg.walk.stability_margin,   cfg.run.stability_margin,  rb);
    eff_walk.max_step_L          = std::lerp(cfg.walk.max_step_L,         cfg.run.max_step_L,        rb);
    eff_walk.d_rear_max          = std::lerp(cfg.walk.d_rear_max,         cfg.run.d_rear_max,        rb);
    eff_walk.xcom_scale          = std::lerp(cfg.walk.xcom_scale,         cfg.run.xcom_scale,        rb);
    eff_walk.leg_flex_coeff      = std::lerp(cfg.walk.leg_flex_coeff,     cfg.run.leg_flex_coeff,    rb);
    eff_walk.bob_scale           = std::lerp(cfg.walk.bob_scale,          cfg.run.bob_scale,         rb);
    eff_walk.bob_amp             = std::lerp(cfg.walk.bob_amp,            cfg.run.bob_amp,           rb);
    eff_walk.h_clear_min_ratio   = std::lerp(cfg.walk.h_clear_min_ratio,  cfg.run.h_clear_min_ratio, rb);
    eff_walk.double_support_time = cfg.walk.double_support_time * (1.0 - rb);
    eff_step.h_clear_ratio       = std::lerp(cfg.step.h_clear_ratio,      cfg.run.h_clear_ratio,     rb);
    eff_physics.accel            = cfg.physics.accel * std::lerp(1.0, cfg.run.accel_factor,          rb);
    eff_physics.walk_max_speed   = std::lerp(cfg.physics.walk_max_speed,  cfg.run.max_speed,         rb);
}

} // namespace

// ── Constructor / reset / snapshot ───────────────────────────────────────────

SimulationCore::SimulationCore(AppConfig& config)
    : m_config(config), m_terrain(config.terrain)
{
    m_terrain.generate();
}

void SimulationCore::reset(const ScenarioInit& init)
{
    if (init.terrain_seed != 0) {
        m_config.terrain.seed = static_cast<int>(init.terrain_seed);
        m_terrain.generate();
    }
    m_state             = SimState{};
    m_state.cm.position = init.cm_pos;
    m_state.cm.velocity = init.cm_vel;
    resetHeadState(m_state.character);
}

void SimulationCore::loadState(const SimState& snap) { m_state = snap; }

void SimulationCore::regenerateTerrain()
{
    m_terrain.generate();
    m_state.character.feet_initialized = false;
    m_state.character.ground_reference_initialized = false;
}

void SimulationCore::teleportCM(double x, double vx)
{
    m_state.cm.position.x = x;
    m_state.cm.velocity.x = vx;
    m_state.character.ground_reference_initialized = false;
    resetHeadState(m_state.character);
}

void SimulationCore::setCMVelocity(Vec2 vel) { m_state.cm.velocity = vel; }

void SimulationCore::toggleFootPin(bool left)
{
    FootState& foot = left ? m_state.character.foot_left : m_state.character.foot_right;
    foot.pinned = !foot.pinned;
    if (foot.pinned) {
        foot.pinned_pos    = foot.pos;
        foot.pinned_normal = foot.ground_normal;
    }
}

void SimulationCore::toggleHandPin(bool left)
{
    CharacterState& ch = m_state.character;
    bool& pinned = left ? ch.hand_left_pinned : ch.hand_right_pinned;
    Vec2& target = left ? ch.hand_left_target : ch.hand_right_target;
    const Vec2 current = left ? ch.hand_left : ch.hand_right;

    pinned = !pinned;
    if (pinned)
        target = current;
}

// ── Main step ────────────────────────────────────────────────────────────────

void SimulationCore::step(double dt, const InputFrame& input)
{
    m_state.sim_time += dt;
    StepCtx ctx;
    stepComputeConstants(ctx);
    stepBootstrapCM(ctx, input);
    stepGroundReference(ctx, dt);
    stepProcessInput(ctx, input);
    stepBlendRunMode(ctx, input, dt);
    stepBlendParams(ctx);
    stepIntegratePhysics(ctx, dt);
    stepReconstructGeometry(ctx, input);
    stepAdvanceSwing(ctx, dt);
    stepComputeTriggerState(ctx);
    stepAirborneJump(ctx);
    stepFireTriggers(ctx);
    stepApplyConstraints(ctx);
    stepWriteOutput(ctx, input, dt);
}

// ── Step sub-phases ───────────────────────────────────────────────────────────

void SimulationCore::stepComputeConstants(StepCtx& ctx)
{
    ctx.L           = m_config.character.body_height_m / 5.0;
    ctx.g           = m_config.physics.gravity;
    ctx.h_nominal   = computeNominalY(ctx.L, m_config.standing.d_pref,
                                      m_config.character.cm_pelvis_ratio);
    ctx.reach_radius = 2.0 * ctx.L;
}

void SimulationCore::stepBootstrapCM(StepCtx& ctx, const InputFrame& input)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    ctx.prev_contact_left = ch.foot_left.on_ground;
    ctx.prev_contact_right = ch.foot_right.on_ground;
    ctx.prev_jump_flight_active = ch.jump_flight_active;

    if (!ch.feet_initialized) {
        cm.position.y = m_terrain.height_at(cm.position.x) + ctx.h_nominal;
        cm.velocity.y = 0.0;
        ch.ground_reference_initialized = false;
        SIM_LOG("Bootstrap: CM=(%.3f, %.3f)\n", cm.position.x, cm.position.y);
    }

    if (input.set_velocity.has_value())
        cm.velocity = *input.set_velocity;

    if (!ch.foot_left.swinging && !ch.foot_right.swinging) {
        ch.recovery_followthrough_active = false;
        ch.recovery_followthrough_dir    = 0.0;
    }
}

void SimulationCore::stepGroundReference(StepCtx& ctx, double dt)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    const Vec2 pelvis_ref = reconstructPelvis(cm, m_config.character, m_config.reconstruction);
    const bool airborne_ref =
        (pelvis_ref.y - m_terrain.height_at(pelvis_ref.x) > 2.0 * ctx.L);

    const GroundReferenceState ground_state =
        updateGroundReference(ch, m_terrain, cm, pelvis_ref, airborne_ref,
                              m_config.terrain_sampling, ctx.L, dt);

    ctx.ref_ground   = ground_state.sample.mean_y;
    ctx.ref_slope    = ground_state.sample.slope;
    ctx.ground_back  = ground_state.sample.back;
    ctx.ground_fwd   = ground_state.sample.fwd;
    ctx.airborne_ref = ground_state.airborne_ref;
    ctx.sin_t = std::sin(std::atan(ctx.ref_slope));
    ctx.cos_t = std::cos(std::atan(ctx.ref_slope));
}

void SimulationCore::stepProcessInput(StepCtx& ctx, const InputFrame& input)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    ctx.input_dir = 0.0;
    if (input.key_right) ctx.input_dir += 1.0;
    if (input.key_left)  ctx.input_dir -= 1.0;

    if (input.jump
        && !ch.jump_preload_active
        && !ch.jump_flight_active
        && !ctx.airborne_ref) {
        beginJumpPreload(ch, ch.run_mode, std::abs(cm.velocity.x), ctx.L, cm, m_config.jump);
    }
}

void SimulationCore::stepBlendRunMode(StepCtx& ctx, const InputFrame& input, double dt)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    {
        const bool want_run = input.key_run
                           && std::abs(cm.velocity.x) > m_config.physics.stop_speed;
        const double run_alpha = 1.0 - std::exp(-dt / std::max(m_config.run.blend_tau, 1.0e-4));
        ch.run_blend += (want_run ? 1.0 - ch.run_blend : -ch.run_blend) * run_alpha;
        ch.run_blend  = std::clamp(ch.run_blend, 0.0, 1.0);
        ch.run_mode   = (ch.run_blend > 0.5);
    }

    // Tie run_phase to swinging foot so vertical oscillation stays in sync with contacts.
    if (ch.run_blend > 0.0) {
        if (ch.foot_right.swinging && !ch.foot_left.swinging) {
            ch.run_phase = 0.5 * std::clamp(ch.foot_right.swing_t, 0.0, 1.0);
        } else if (ch.foot_left.swinging && !ch.foot_right.swinging) {
            ch.run_phase = 0.5 + 0.5 * std::clamp(ch.foot_left.swing_t, 0.0, 1.0);
        } else {
            ch.run_phase = ch.run_last_touchdown_left ? 0.0 : 0.5;
        }
    }

    ctx.rb = ch.run_blend;
}

void SimulationCore::stepBlendParams(StepCtx& ctx)
{
    ctx.speed_abs  = std::abs(m_state.cm.velocity.x);
    ctx.run_timing = computeRunTimingTargets(m_config.run, ctx.speed_abs,
                                             m_config.run.max_speed, ctx.L);
    ctx.eff_walk    = m_config.walk;
    ctx.eff_step    = m_config.step;
    ctx.eff_physics = m_config.physics;

    if (ctx.rb > 0.0)
        blendWalkRunConfig(ctx.eff_walk, ctx.eff_step, ctx.eff_physics, m_config, ctx.rb);

    ctx.max_spd = ctx.eff_physics.walk_max_speed;
}

void SimulationCore::stepIntegratePhysics(StepCtx& ctx, double dt)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    // Horizontal
    integrateHorizontalMotion(cm, ch, ctx.eff_physics, ctx.input_dir,
                              ctx.sin_t, ctx.cos_t, ctx.g, ctx.airborne_ref,
                              ctx.max_spd, dt, ctx.accel);
    if (std::abs(cm.velocity.x) > m_config.physics.stop_speed)
        ch.facing = std::copysign(1.0, cm.velocity.x);

    // Downhill crouch
    updateDownhillCrouch(ch, ctx.eff_walk, ctx.eff_physics, ctx.ref_slope,
                         cm.velocity.x, ctx.max_spd, dt);

    // Height target
    const HeightTargetState height_state =
        computeHeightTargetState(ch, cm, ctx.eff_walk, ctx.eff_physics,
                                 ctx.ref_ground, ctx.ref_slope, ctx.h_nominal,
                                 m_config.character.cm_pelvis_ratio, ctx.L);
    ctx.h_ip       = height_state.h_ip;
    ctx.speed_drop = height_state.speed_drop;
    ctx.slope_drop = height_state.slope_drop;
    ctx.y_tgt      = height_state.y_tgt;

    // Jump preload crouch adjustment
    if (ch.jump_preload_active) {
        ch.jump_preload_t += dt;
        const double preload_progress = smooth01(
            ch.jump_preload_t / std::max(ch.jump_preload_duration, 1.0e-4));
        ctx.y_tgt -= ch.jump_preload_depth * preload_progress;
    }

    const bool jump_takeoff_now = ch.jump_preload_active
                               && ch.jump_preload_t >= ch.jump_preload_duration;
    ctx.jump_vertical_override = ch.jump_preload_active || ch.jump_flight_active
                               || jump_takeoff_now;

    // Running height target: phase oscillation tied to footfalls.
    // run_phase 0/0.5 = alternating touchdowns → oscillate at step frequency (4π*phase).
    if (ctx.rb > 0.0 && !ctx.jump_vertical_override) {
        const double run_compression = std::lerp(0.09 * ctx.L, 0.14 * ctx.L,
                                                 ctx.run_timing.speed_ratio);
        const double run_bob         = -ctx.eff_walk.bob_amp * ctx.L
                                     * std::cos(4.0 * kPi * ch.run_phase);
        const double y_tgt_run       = ctx.ref_ground + ctx.h_nominal
                                     - run_compression + run_bob
                                     + ctx.eff_walk.cm_height_offset
                                     - ctx.speed_drop - ctx.slope_drop;
        ctx.y_tgt = std::lerp(ctx.y_tgt, y_tgt_run, ctx.rb);
    }

    // Landing recovery: boost step speed and tighten support window
    if (ch.landing_recovery_timer > 0.0)
        ch.landing_recovery_timer = std::max(0.0, ch.landing_recovery_timer - dt);

    const double landing_recovery_alpha = (ch.landing_recovery_timer > 0.0)
        ? smooth01(ch.landing_recovery_timer / m_config.jump.landing_dur_jump)
        : 0.0;
    ctx.landing_recovery_gain   = ch.landing_recovery_boost * landing_recovery_alpha;
    ctx.landing_recovery_active = ctx.landing_recovery_gain > 0.0;
    if (ctx.landing_recovery_active) {
        ctx.eff_walk.step_speed          *= (1.0 + 0.75 * ctx.landing_recovery_gain);
        ctx.eff_walk.d_rear_max          *= std::max(0.55, 1.0 - 0.25 * ctx.landing_recovery_gain);
        ctx.eff_walk.double_support_time *= std::max(0.0,  1.0 - 0.80 * ctx.landing_recovery_gain);
    }

    // Running downward spring: symmetric correction when CM is above target
    if (ctx.rb > 0.0 && !ctx.jump_vertical_override) {
        const double delta = ctx.y_tgt - cm.position.y;
        if (delta < 0.0) {
            const double vy_w = ctx.eff_physics.vy_max
                              * std::tanh(delta / ctx.eff_physics.d_soft);
            ctx.accel.y += ctx.rb * std::min(0.0, (vy_w - cm.velocity.y) * ctx.eff_physics.vy_tau);
        }
    }

    // Jump preload takeoff: apply impulse before vertical integration
    if (ch.jump_preload_active && ch.jump_preload_t >= ch.jump_preload_duration) {
        cm.velocity.y = std::max(cm.velocity.y, m_config.physics.jump_impulse);
        ctx.eff_physics.spring_enabled = false;
    }
    if (ch.jump_flight_active)
        ctx.eff_physics.spring_enabled = false;

    ctx.pre_vertical_velocity = cm.velocity.y;
    integrateVerticalMotion(cm, ctx.accel, m_terrain, m_config.character,
                            m_config.reconstruction, ctx.eff_physics,
                            ctx.y_tgt, ctx.ref_ground, ctx.L, ctx.g, dt);
}

void SimulationCore::stepReconstructGeometry(StepCtx& ctx, const InputFrame& input)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    ctx.pelvis       = reconstructPelvis(cm, m_config.character, m_config.reconstruction);
    // Recompute airborne from final integrated position so a velocity impulse
    // (e.g. right-click drag) is detected in the same frame it shoots the CM up.
    ctx.airborne_final = (ctx.pelvis.y - m_terrain.height_at(ctx.pelvis.x) > 2.0 * ctx.L);

    if (!ch.feet_initialized)
        initializeFeetUnderPelvis(ch, m_terrain, ctx.pelvis, ctx.L, m_config.standing.d_pref);

    if (ch.jump_preload_active && ch.jump_preload_t >= ch.jump_preload_duration) {
        ch.jump_preload_active = false;
        cm.velocity.y = std::max(cm.velocity.y, m_config.physics.jump_impulse);
        beginAirborneLandingProtocol(ch, m_config, m_terrain, m_config.reconstruction,
                                     cm, ctx.g, ctx.L);
    }

    applyPinnedFootPositions(ch);
    applyDraggedFootPositions(ch, input);
}

void SimulationCore::stepAdvanceSwing(StepCtx& ctx, double dt)
{
    CharacterState& ch = m_state.character;

    ctx.was_swinging_L = ch.foot_left.swinging;
    ctx.was_swinging_R = ch.foot_right.swinging;

    stepRetargetLateSwings(ctx);

    advanceSwingFoot(ch.foot_left,  m_terrain, ctx.pelvis, dt, ctx.reach_radius, ctx.eff_walk);
    advanceSwingFoot(ch.foot_right, m_terrain, ctx.pelvis, dt, ctx.reach_radius, ctx.eff_walk);
    ctx.heel_strike_L = ctx.was_swinging_L && !ch.foot_left.swinging;
    ctx.heel_strike_R = ctx.was_swinging_R && !ch.foot_right.swinging;
    if (ctx.heel_strike_L) ch.run_last_touchdown_left = true;
    if (ctx.heel_strike_R) ch.run_last_touchdown_left = false;

    // Double-support: force minimum cooldown on heel-strike (blends to 0 in run mode).
    if ((ctx.heel_strike_L || ctx.heel_strike_R) && ctx.eff_walk.double_support_time > 0.0)
        ch.step_cooldown = std::max(ch.step_cooldown, ctx.eff_walk.double_support_time);

    if (ch.step_cooldown > 0.0) ch.step_cooldown -= dt;
}

void SimulationCore::stepComputeTriggerState(StepCtx& ctx)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    ctx.omega0 = std::sqrt(ctx.g / ctx.h_nominal);
    ctx.xi     = cm.position.x + ctx.eff_walk.xcom_scale * cm.velocity.x / ctx.omega0;

    // Use swing target while mid-arc: trigger sees where foot *will* land, not current arc pos.
    ctx.eff_lx = ch.foot_left.swinging  ? ch.foot_left.swing_target.x  : ch.foot_left.pos.x;
    ctx.eff_rx = ch.foot_right.swinging ? ch.foot_right.swing_target.x : ch.foot_right.pos.x;
}

void SimulationCore::stepAirborneJump(StepCtx& ctx)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    // When airborne without jump protocol: start landing prediction so feet track target.
    if (ctx.airborne_final && !ch.jump_flight_active) {
        beginAirborneLandingProtocol(ch, m_config, m_terrain, m_config.reconstruction,
                                     cm, ctx.g, ctx.L);
    }

    stepHandleJumpFlight(ctx);
    stepHandleGroundRecontact(ctx);

    ctx.any_swinging = ch.foot_left.swinging || ch.foot_right.swinging;
}

void SimulationCore::stepFireTriggers(StepCtx& ctx)
{
    CharacterState& ch = m_state.character;

    if (!ch.jump_preload_active && !ch.jump_flight_active
        && ch.run_mode && ch.feet_initialized
        && !ctx.any_swinging && !ctx.airborne_final)
        stepFireRunTrigger(ctx);
    else if (!ch.jump_preload_active && !ch.jump_flight_active
             && ch.feet_initialized && ch.step_cooldown <= 0.0
             && !ctx.any_swinging && !ctx.airborne_final)
        stepFireWalkTrigger(ctx);
}

void SimulationCore::stepApplyConstraints(StepCtx& ctx)
{
    CharacterState& ch = m_state.character;

    ctx.was_grounded_L = ch.foot_left.on_ground;
    ctx.was_grounded_R = ch.foot_right.on_ground;

    applyFootConstraints(ch, m_terrain, ctx.pelvis, ctx.reach_radius);

    tryRecoveryStepOnLiftOff(ch, ctx.airborne_final, ctx.any_swinging,
                             ctx.was_grounded_L, ctx.was_grounded_R,
                             ctx.input_dir,
                             [&](bool l, bool c) { stepLaunchSwing(l, c, ctx); });

    unpinLiftedFeet(ch, ctx.airborne_final, ctx.was_grounded_L, ctx.was_grounded_R);
}

void SimulationCore::stepWriteOutput(StepCtx& ctx, const InputFrame& input, double dt)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    stepUpdateContactEvents(ctx);
    stepUpdateSlideEvents(dt);

    cacheXCoMState(m_state, ch, m_state.cm, ctx.eff_walk, ctx.eff_lx, ctx.eff_rx, ctx.xi, ctx.L);

    ch.debug_on_floor    = !ctx.airborne_final;
    ch.debug_cm_target_y = ctx.y_tgt;
    ch.debug_ref_ground  = ctx.ref_ground;
    ch.debug_ref_slope   = ctx.ref_slope;
    ch.debug_h_ip        = ctx.h_ip;
    ch.debug_speed_drop  = ctx.speed_drop;
    ch.debug_slope_drop  = ctx.slope_drop;
    ch.debug_cm_offset   = ctx.eff_walk.cm_height_offset;
    ch.debug_ground_back = ctx.ground_back;
    ch.debug_ground_fwd  = ctx.ground_fwd;

    CharacterReconstructionConfig eff_reconstruction = m_config.reconstruction;
    if (ctx.rb > 0.0)
        eff_reconstruction.theta_max_deg = std::lerp(m_config.reconstruction.theta_max_deg,
                                                     m_config.run.theta_max_deg, ctx.rb);
    updateCharacterState(ch, cm, m_config.character, eff_reconstruction,
                         !ctx.airborne_final, ch.run_mode, dt, ctx.ref_slope);

    UpperBodyControl upper_body;
    upper_body.input_dir = ctx.input_dir;
    if (ch.hand_left_pinned)  upper_body.targets.left_hand_target  = ch.hand_left_target;
    if (ch.hand_right_pinned) upper_body.targets.right_hand_target = ch.hand_right_target;
    if (input.hand_left_drag) {
        upper_body.targets.left_hand_target = input.hand_left_pos;
        if (ch.hand_left_pinned) ch.hand_left_target = input.hand_left_pos;
    }
    if (input.hand_right_drag) {
        upper_body.targets.right_hand_target = input.hand_right_pos;
        if (ch.hand_right_pinned) ch.hand_right_target = input.hand_right_pos;
    }
    upper_body.targets.gaze_target_world = input.gaze_target_world;
    updateUpperBodyState(ch, cm, m_config.character, m_config.physics,
                         m_config.walk, m_config.arms, m_config.head,
                         upper_body, dt);
}

void SimulationCore::stepRetargetLateSwings(StepCtx& ctx)
{
    if (!ctx.landing_recovery_active) return;

    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    auto retargetSwingIfLate = [&](FootState& swing_foot, const FootState& stance_foot) {
        if (!swing_foot.swinging) return;
        const double t_remaining  = estimateSwingRemainingTime(swing_foot, ctx.eff_walk);
        const double future_cm_x  = cm.position.x + cm.velocity.x * t_remaining;
        const double old_target_x = swing_foot.swing_target.x;
        const double new_target_x = retargetLandingRecoveryX(
            old_target_x, ch, stance_foot, ctx.pelvis, future_cm_x,
            ctx.reach_radius, ctx.L, ctx.ref_slope, ctx.eff_walk, ctx.landing_recovery_gain);
        if (std::abs(new_target_x - old_target_x) <= 1.0e-5) return;
        swing_foot.swing_target.x = new_target_x;
        swing_foot.swing_target.y = m_terrain.height_at(new_target_x);
        refreshSwingArcProfile(swing_foot, m_terrain, ctx.L, ctx.eff_step, ctx.eff_walk,
                               ctx.speed_abs, ctx.max_spd);
    };

    retargetSwingIfLate(ch.foot_left,  ch.foot_right);
    retargetSwingIfLate(ch.foot_right, ch.foot_left);
}

void SimulationCore::stepHandleJumpFlight(StepCtx& ctx)
{
    if (!m_state.character.jump_flight_active) return;

    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    updateJumpLandingTargets(ch, m_config, m_terrain, m_config.reconstruction,
                             cm, ctx.g, ctx.L);
    const double progress = 1.0 - ch.jump_time_remaining
                                  / std::max(ch.jump_total_flight_time, 1.0e-4);
    updateJumpFeetInFlight(ch, ctx.pelvis, progress);

    if (ctx.airborne_final) {
        releaseFeetAirborne(ch);
        ch.foot_left.airborne  = true;
        ch.foot_right.airborne = true;
    }
}

void SimulationCore::stepHandleGroundRecontact(StepCtx& ctx)
{
    CharacterState& ch = m_state.character;

    if (ctx.airborne_final) {
        if (!ch.jump_flight_active)
            releaseFeetAirborne(ch);
        return;
    }
    if (!ctx.airborne_ref || ctx.airborne_final == ctx.airborne_ref) return;

    const double impact_speed = std::max(0.0, -ctx.pre_vertical_velocity);
    const double impact_ratio = std::clamp(
        impact_speed / std::max(m_config.physics.jump_impulse, 1.0e-4), 0.0, 2.0);

    if (ch.jump_flight_active && ch.jump_targets_valid) {
        ch.foot_left.pos        = ch.jump_left_target;
        ch.foot_left.pinned     = true;
        ch.foot_left.pinned_pos = ch.jump_left_target;
        ch.foot_left.swinging   = false;
        ch.foot_left.airborne   = false;
        ch.foot_left.on_ground  = true;

        ch.foot_right.pos        = ch.jump_right_target;
        ch.foot_right.pinned     = true;
        ch.foot_right.pinned_pos = ch.jump_right_target;
        ch.foot_right.swinging   = false;
        ch.foot_right.airborne   = false;
        ch.foot_right.on_ground  = true;
        ch.jump_flight_active    = false;
        ch.jump_preload_active   = false;
        ch.jump_targets_valid    = false;
        ch.landing_recovery_timer = m_config.jump.landing_dur_jump;
        ch.landing_recovery_boost = m_config.jump.landing_boost_base_jump
                                  + m_config.jump.landing_boost_scale_jump * impact_ratio;
        return;
    }

    bootstrapFeetOnLanding(ch, m_config.standing, m_terrain,
                           ctx.pelvis, ctx.reach_radius, ctx.L);
    ch.landing_recovery_timer = m_config.jump.landing_dur_walk;
    ch.landing_recovery_boost = m_config.jump.landing_boost_base_walk
                              + m_config.jump.landing_boost_scale_walk * impact_ratio;
}

void SimulationCore::stepFireRunTrigger(StepCtx& ctx)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    const bool back_is_left = (ch.facing >= 0.0) ? (ch.foot_left.pos.x < ch.foot_right.pos.x)
                                                 : (ch.foot_left.pos.x > ch.foot_right.pos.x);
    FootState& back_foot  = back_is_left ? ch.foot_left : ch.foot_right;
    FootState& front_foot = (&back_foot == &ch.foot_left) ? ch.foot_right : ch.foot_left;

    const double back_extent      = std::lerp(0.40 * ctx.L, 0.80 * ctx.L, ctx.run_timing.speed_ratio);
    const double movement_back_x  = ctx.pelvis.x - ch.facing * back_extent;
    const bool   outside_back_win = (back_foot.pos.x - movement_back_x) * ch.facing < 0.0;
    const double behind_back      = (ctx.pelvis.x - back_foot.pos.x) * ch.facing;
    const double trigger_dist     = std::lerp(0.45 * ctx.L, 0.75 * ctx.L, ctx.run_timing.speed_ratio);
    if (!outside_back_win && behind_back <= trigger_dist) return;

    const double tx = computeRunLandingX(m_config.run, ch, front_foot,
                                         ctx.pelvis, cm.velocity.x,
                                         ctx.reach_radius, ctx.L, ctx.run_timing,
                                         ctx.ref_slope);
    beginSwingStep(back_foot, front_foot, ch, tx, m_terrain,
                   false, ctx.L, ctx.eff_step, ctx.eff_walk, ctx.speed_abs, ctx.max_spd);
    const double uphill_alignment = ch.facing * ctx.ref_slope;
    const double step_rise = back_foot.swing_target.y - back_foot.swing_start.y;
    if (uphill_alignment > 0.0 && step_rise > 0.0) {
        const double uphill_strength = std::clamp(uphill_alignment / 0.35, 0.0, 1.0);
        back_foot.swing_h_clear += std::lerp(0.0, 0.16 * ctx.L, uphill_strength);
    }
    ch.step_cooldown = 0.0;
}

void SimulationCore::stepFireWalkTrigger(StepCtx& ctx)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    const StepTriggerEval trigger_eval =
        evaluateStepTriggers(ch, ctx.eff_lx, ctx.eff_rx, ctx.xi, cm.velocity.x,
                             ctx.eff_walk.eps_step, ctx.pelvis, ctx.L,
                             ctx.eff_walk.d_rear_max);

    if (!trigger_eval.xcom_trigger && !trigger_eval.rear_trigger) return;

    const bool no_forward_input = std::abs(ctx.input_dir) <= kInputDirDeadzone;
    const bool corrective       = no_forward_input
                               && trigger_eval.rear_trigger
                               && !trigger_eval.xcom_trigger;
    stepLaunchSwing(trigger_eval.step_left_xcom, corrective, ctx);
    SIM_LOG("Step trigger: %s  d_rear=%.2fL  xcom=%d  rear=%d\n",
            trigger_eval.step_left_xcom ? "LEFT" : "RIGHT",
            trigger_eval.d_rear / ctx.L,
            static_cast<int>(trigger_eval.xcom_trigger),
            static_cast<int>(trigger_eval.rear_trigger));
}

void SimulationCore::stepUpdateContactEvents(StepCtx& ctx)
{
    CharacterState& ch = m_state.character;
    SimEvents&      events = m_state.events;
    events.left_touchdown  = !ctx.prev_contact_left && ch.foot_left.on_ground;
    events.right_touchdown = !ctx.prev_contact_right && ch.foot_right.on_ground;
    events.landed_from_jump = ctx.prev_jump_flight_active && !ch.jump_flight_active
                           && (events.left_touchdown || events.right_touchdown);
}

void SimulationCore::stepUpdateSlideEvents(double dt)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    SimEvents&      events = m_state.events;

    ch.left_slide_emit_timer = std::max(0.0, ch.left_slide_emit_timer - dt);
    ch.right_slide_emit_timer = std::max(0.0, ch.right_slide_emit_timer - dt);

    const bool left_slide_active_now = isSlideActive(ch.foot_left, cm.velocity.x);
    const bool right_slide_active_now = isSlideActive(ch.foot_right, cm.velocity.x);
    events.left_slide_active = left_slide_active_now && ch.left_slide_emit_timer <= 0.0;
    events.right_slide_active = right_slide_active_now && ch.right_slide_emit_timer <= 0.0;

    if (events.left_slide_active)
        ch.left_slide_emit_timer = m_config.particles.slide_emit_interval_s;
    if (events.right_slide_active)
        ch.right_slide_emit_timer = m_config.particles.slide_emit_interval_s;
    if (!left_slide_active_now)
        ch.left_slide_emit_timer = 0.0;
    if (!right_slide_active_now)
        ch.right_slide_emit_timer = 0.0;
}

bool SimulationCore::stepLaunchSwing(bool step_left, bool corrective, StepCtx& ctx)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    FootState& step_foot   = step_left ? ch.foot_left  : ch.foot_right;
    FootState& stance_foot = step_left ? ch.foot_right : ch.foot_left;
    if (step_foot.swinging) return false;

    double tx = computeStepLandingX(ctx.eff_walk, ch, stance_foot,
                                    ctx.pelvis, ctx.xi, ctx.L, ctx.reach_radius, ctx.ref_slope);
    if (ctx.landing_recovery_active) {
        FootState predicted = step_foot;
        predicted.swing_start  = step_foot.pos;
        predicted.swing_target = { tx, m_terrain.height_at(tx) };
        predicted.swing_t      = 0.0;
        refreshSwingArcProfile(predicted, m_terrain, ctx.L, ctx.eff_step, ctx.eff_walk,
                               ctx.speed_abs, ctx.max_spd);
        const double t_remaining = estimateSwingRemainingTime(predicted, ctx.eff_walk);
        const double future_cm_x = cm.position.x + cm.velocity.x * t_remaining;
        tx = retargetLandingRecoveryX(tx, ch, stance_foot, ctx.pelvis, future_cm_x,
                                      ctx.reach_radius, ctx.L, ctx.ref_slope, ctx.eff_walk,
                                      ctx.landing_recovery_gain);
    }
    beginSwingStep(step_foot, stance_foot, ch, tx, m_terrain,
                   corrective, ctx.L, ctx.eff_step, ctx.eff_walk, ctx.speed_abs, ctx.max_spd);
    return true;
}
