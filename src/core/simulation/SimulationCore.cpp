#include "core/simulation/SimulationCore.h"

#include "core/character/CharacterState.h"
#include "core/character/ArmController.h"
#include "core/character/HeadController.h"
#include "core/character/UpperBodyKinematics.h"
#include "core/physics/Geometry.h"
#include "core/simulation/SimVerbosity.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#define SIM_LOG(...) do { if (g_sim_verbose) std::fprintf(stderr, __VA_ARGS__); } while (0)

static constexpr double kPi       = 3.14159265358979323846;
static constexpr double kDegToRad = kPi / 180.0;

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
    if (dist > r && dist > 1e-9)
        return { center.x + dx * (r / dist),
                 center.y + dy * (r / dist) };
    return pos;
}

double terrainDistSqToPelvis(const Terrain& terrain, const Vec2& pelvis, double x)
{
    const double y  = terrain.height_at(x);
    const double dx = x - pelvis.x;
    const double dy = y - pelvis.y;
    return dx * dx + dy * dy;
}

bool terrainPointInsideDisk(const Terrain& terrain,
                            const Vec2&    pelvis,
                            double         radius,
                            double         x)
{
    return terrainDistSqToPelvis(terrain, pelvis, x) <= radius * radius;
}

double clampTerrainEndpointX(const Terrain& terrain,
                             const Vec2&    pelvis,
                             double         radius,
                             double         x_start,
                             double         x_target)
{
    const double r_sq = radius * radius;
    auto inside = [&](double x) {
        return terrainDistSqToPelvis(terrain, pelvis, x) <= r_sq;
    };

    if (!inside(x_start)) return x_start;
    if (inside(x_target)) return x_target;

    double x_valid = x_start;
    constexpr int coarse_steps = 32;
    for (int i = 1; i <= coarse_steps; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(coarse_steps);
        const double x = x_start + (x_target - x_start) * t;
        if (!inside(x)) {
            double x_invalid = x;
            for (int j = 0; j < 28; ++j) {
                const double xm = 0.5 * (x_valid + x_invalid);
                if (inside(xm)) x_valid = xm;
                else            x_invalid = xm;
            }
            return x_valid;
        }
        x_valid = x;
    }

    return x_valid;
}

double slideTerrainEndpointX(const Terrain& terrain,
                             const Vec2&    pelvis,
                             double         radius,
                             double         x_prev,
                             double         x_target,
                             double         x_anchor,
                             double         alpha)
{
    double x_start = x_prev;
    if (!terrainPointInsideDisk(terrain, pelvis, radius, x_start)) {
        if (terrainPointInsideDisk(terrain, pelvis, radius, x_anchor))
            x_start = clampTerrainEndpointX(terrain, pelvis, radius, x_anchor, x_start);
        else
            return x_prev;
    }

    if (std::abs(x_target - x_start) <= 1.0e-9)
        return x_start;

    const double x_step = x_start + (x_target - x_start) * alpha;
    if (terrainPointInsideDisk(terrain, pelvis, radius, x_step))
        return x_step;
    return clampTerrainEndpointX(terrain, pelvis, radius, x_start, x_step);
}

struct GroundReferenceSample {
    Vec2 back;
    Vec2 fwd;
    double mean_y = 0.0;
    double slope  = 0.0;
};

struct GroundReferenceState {
    Vec2 pelvis_ref;
    bool airborne_ref = false;
    GroundReferenceSample sample;
};

struct HeightTargetState {
    double h_ip = 0.0;
    double speed_drop = 0.0;
    double slope_drop = 0.0;
    double y_tgt = 0.0;
};

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

    // ── Swing speed scale ────────────────────────────────────────────────────
    // A step with large vertical drop/rise covers more total arc distance than
    // its horizontal extent alone. Scale down the parameter advance rate so the
    // foot takes proportionally longer — downhill steps slow the swing naturally.
    {
        const double dx       = tx - step_foot.swing_start.x;
        const double dy       = step_foot.swing_target.y - step_foot.swing_start.y;
        const double hdist    = std::abs(dx);
        const double arc_len  = std::sqrt(dx * dx + dy * dy);
        step_foot.swing_speed_scale = (arc_len > 1e-9)
            ? std::max(0.30, hdist / arc_len)
            : 1.0;
    }

    // ── Dynamic foot lift (h_clear) ──────────────────────────────────────────
    // Base lift from StepConfig. Uphill adds lift (foot must clear rising terrain);
    // downhill reduces it (terrain drops away, less clearance needed).
    // Speed adds a small bonus for larger, faster arcs.
    {
        const double dx         = tx - step_foot.swing_start.x;
        const double dy         = step_foot.swing_target.y - step_foot.swing_start.y;
        const double hdist      = std::abs(dx);
        const double step_slope = (hdist > 1e-9) ? (dy / hdist) : 0.0;  // +uphill / −downhill
        const double h_base     = step_cfg.h_clear_ratio * L;
        const double h_slope    = walk_cfg.h_clear_slope_factor * L * step_slope;
        const double h_speed    = walk_cfg.h_clear_speed_factor * L
                                * std::clamp(speed_abs / std::max(walk_max_speed, 1e-4),
                                             0.0, 1.0);
        step_foot.swing_h_clear = std::max(walk_cfg.h_clear_min_ratio * L,
                                           h_base + h_slope + h_speed);
    }

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

    ch.foot_right.pos        = { rx, terrain.height_at(rx) };
    ch.foot_right.pinned     = true;
    ch.foot_right.pinned_pos = ch.foot_right.pos;
    ch.foot_right.swinging   = false;
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
    if (!ch.foot_left.swinging)  applyGroundConstraint(ch.foot_left,  terrain);
    if (!ch.foot_right.swinging) applyGroundConstraint(ch.foot_right, terrain);

    // Constraint 2: 2L reach circle — global, always enforced.
    ch.foot_left.pos  = clampToCircle(ch.foot_left.pos,  pelvis, reach_radius);
    ch.foot_right.pos = clampToCircle(ch.foot_right.pos, pelvis, reach_radius);

    if (!ch.foot_left.swinging)  applyGroundConstraint(ch.foot_left,  terrain);
    if (!ch.foot_right.swinging) applyGroundConstraint(ch.foot_right, terrain);
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
        launchStep(true, std::abs(input_dir) <= 0.01);
        SIM_LOG("Step trigger: LEFT  lift_off=1\n");
    } else if (lift_off_R && ch.foot_left.on_ground) {
        launchStep(false, std::abs(input_dir) <= 0.01);
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

void writeDebugState(CharacterState&             ch,
                     bool                        airborne_final,
                     double                      y_tgt,
                     const GroundReferenceSample& ground_ref,
                     double                      ref_slope,
                     double                      h_ip,
                     double                      speed_drop,
                     double                      slope_drop,
                     double                      cm_height_offset)
{
    ch.debug_on_floor    = !airborne_final;
    ch.debug_cm_target_y = y_tgt;
    ch.debug_ref_ground  = ground_ref.mean_y;
    ch.debug_ref_slope   = ref_slope;
    ch.debug_h_ip        = h_ip;
    ch.debug_speed_drop  = speed_drop;
    ch.debug_slope_drop  = slope_drop;
    ch.debug_cm_offset   = cm_height_offset;
    ch.debug_ground_back = ground_ref.back;
    ch.debug_ground_fwd  = ground_ref.fwd;
}

GroundReferenceSample computeGroundReferenceSample(const Terrain& terrain,
                                                   const Vec2&    pelvis,
                                                   double         cm_x,
                                                   double         facing,
                                                   double         speed_x,
                                                   double         L,
                                                   const TerrainSamplingConfig& ts,
                                                   double         dt,
                                                   double         left_prev_x,
                                                   double         right_prev_x)
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
    const double span          = std::abs(fwd.x - back.x);
    const double slope         = (span > 1.0e-9)
                               ? facing * (fwd.y - back.y) / span
                               : 0.0;
    return {
        back,
        fwd,
        (back.y + fwd.y) * 0.5,
        slope
    };
}

GroundReferenceState updateGroundReference(CharacterState&                     ch,
                                           const Terrain&                      terrain,
                                           const CMState&                      cm,
                                           const CharacterConfig&              char_cfg,
                                           const CharacterReconstructionConfig& recon_cfg,
                                           const TerrainSamplingConfig&        ts,
                                           double                              L,
                                           double                              dt)
{
    GroundReferenceState state;
    state.pelvis_ref = reconstructPelvis(cm, char_cfg, recon_cfg);
    state.airborne_ref = (state.pelvis_ref.y - terrain.height_at(state.pelvis_ref.x) > 2.0 * L);

    if (!ch.ground_reference_initialized || state.airborne_ref) {
        // Reset endpoints to current x when airborne so that on landing the
        // reference starts from below the character, not from the launch point.
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

    if (!airborne_pre && std::abs(input_dir) > 0.01) {
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
        && (std::abs(input_dir) > 0.01
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

    if (std::abs(input_dir) > 0.01
        && cm.velocity.x * input_dir > 0.0
        && std::abs(cm.velocity.x) > max_spd) {
        cm.velocity.x = std::copysign(max_spd, input_dir);
    } else if (std::abs(input_dir) <= 0.01
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
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    m_state.sim_time  += dt;
    Vec2 accel = {0.0, 0.0};

    // ── Derived constants ────────────────────────────────────────────────────
    const double H         = m_config.character.body_height_m;
    const double L         = H / 5.0;
    const double g         = m_config.physics.gravity;
    const double d_pref    = m_config.standing.d_pref;
    const double h_nominal = computeNominalY(L, d_pref, m_config.character.cm_pelvis_ratio);

    // ── Bootstrap ────────────────────────────────────────────────────────────
    if (!ch.feet_initialized) {
        cm.position.y = m_terrain.height_at(cm.position.x) + h_nominal;
        cm.velocity.y = 0.0;
        ch.ground_reference_initialized = false;
        SIM_LOG("Bootstrap: CM=(%.3f, %.3f)\n", cm.position.x, cm.position.y);
    }

    // ── Velocity override ────────────────────────────────────────────────────
    if (input.set_velocity.has_value())
        cm.velocity = *input.set_velocity;

    if (!ch.foot_left.swinging && !ch.foot_right.swinging) {
        ch.recovery_followthrough_active = false;
        ch.recovery_followthrough_dir    = 0.0;
    }

    // ── Ground reference: asymmetric 2-point average ─────────────────────────
    // Forward window grows with speed (look further ahead when moving fast).
    // Backward window is fixed. Both oriented by facing direction so the line
    // always points in the direction of travel.
    const GroundReferenceState ground_state =
        updateGroundReference(ch, m_terrain, cm, m_config.character,
                              m_config.reconstruction, m_config.terrain_sampling,
                              L, dt);
    const GroundReferenceSample& ground_ref = ground_state.sample;
    const double ref_ground = ground_ref.mean_y;
    const double ref_slope  = ground_ref.slope;

    // ── Terrain slope at CM ──────────────────────────────────────────────────
    const double sin_t = std::sin(std::atan(ref_slope));
    const double cos_t = std::cos(std::atan(ref_slope));

    // ── Input ────────────────────────────────────────────────────────────────
    double input_dir = 0.0;
    if (input.key_right) input_dir += 1.0;
    if (input.key_left)  input_dir -= 1.0;

    // ── Airborne test (pelvis > 2L above terrain) ────────────────────────────
    const bool   airborne_pre = ground_state.airborne_ref;

    // ── Horizontal physics ────────────────────────────────────────────────────
    const double max_spd = m_config.physics.walk_max_speed;
    integrateHorizontalMotion(cm, ch, m_config.physics, input_dir, sin_t, cos_t,
                              g, airborne_pre, max_spd, dt, accel);

    // ── Facing ────────────────────────────────────────────────────────────────
    if (std::abs(cm.velocity.x) > m_config.physics.stop_speed)
        ch.facing = std::copysign(1.0, cm.velocity.x);

    // ── Downhill crouch / reach state ────────────────────────────────────────
    // Driven by actual motion over the slope, not by facing or input, so a turn
    // on a slope does not instantly drop the body. The same filtered state then
    // feeds both CoM height and step length.
    updateDownhillCrouch(ch, m_config.walk, m_config.physics, ref_slope,
                         cm.velocity.x, max_spd, dt);

    // ── Vertical physics ──────────────────────────────────────────────────────
    const HeightTargetState height_state =
        computeHeightTargetState(ch, cm, m_config.walk, m_config.physics,
                                 ref_ground, ref_slope, h_nominal,
                                 m_config.character.cm_pelvis_ratio, L);
    const double h_ip = height_state.h_ip;
    const double speed_drop = height_state.speed_drop;
    const double slope_drop = height_state.slope_drop;
    const double y_tgt = height_state.y_tgt;

    integrateVerticalMotion(cm, accel, m_terrain, m_config.character,
                            m_config.reconstruction, m_config.physics,
                            y_tgt, ref_ground, L, g, dt);

    // ── Pelvis (for foot constraints) — post-integration, authoritative ──────
    const Vec2   pelvis         = reconstructPelvis(cm, m_config.character, m_config.reconstruction);
    const double reach_radius   = 2.0 * L;
    // Recompute airborne using the final integrated position so a velocity impulse
    // (e.g. right-click drag) is detected in the same frame it shoots the CM up.
    const bool   airborne_final = (pelvis.y - m_terrain.height_at(pelvis.x) > 2.0 * L);

    // ── Bootstrap feet ───────────────────────────────────────────────────────
    if (!ch.feet_initialized) {
        initializeFeetUnderPelvis(ch, m_terrain, pelvis, L, m_config.standing.d_pref);
    }

    // ── Pinned constraint (second priority — applied before 2L clamp) ────────
    applyPinnedFootPositions(ch);

    // ── Drag input: position foot at mouse world position ────────────────────
    applyDraggedFootPositions(ch, input);

    // ── Swing arc update ──────────────────────────────────────────────────────
    // h_clear and swing_speed_scale are stored per-foot (set in beginSwingStep).
    const bool was_swinging_L = ch.foot_left.swinging;
    const bool was_swinging_R = ch.foot_right.swinging;

    advanceSwingFoot(ch.foot_left,  m_terrain, pelvis, dt, reach_radius, m_config.walk);
    advanceSwingFoot(ch.foot_right, m_terrain, pelvis, dt, reach_radius, m_config.walk);

    // ── Double-support enforcement ────────────────────────────────────────────
    // When a foot lands (heel-strike), force a minimum cooldown before the next
    // step fires. This guarantees at least double_support_time seconds where
    // both feet are on the ground before the next swing starts.
    {
        const bool heel_strike_L = was_swinging_L && !ch.foot_left.swinging;
        const bool heel_strike_R = was_swinging_R && !ch.foot_right.swinging;
        if ((heel_strike_L || heel_strike_R) && m_config.walk.double_support_time > 0.0)
            ch.step_cooldown = std::max(ch.step_cooldown,
                                        m_config.walk.double_support_time);
    }

    // ── Step trigger helpers ──────────────────────────────────────────────────
    if (ch.step_cooldown > 0.0) ch.step_cooldown -= dt;

    const double omega0 = std::sqrt(g / h_nominal);
    const double xi     = cm.position.x + m_config.walk.xcom_scale * cm.velocity.x / omega0;

    // Effective foot x: use swing target while mid-arc so the trigger sees
    // where each foot *will* land, not its current arc position.
    const double eff_lx = ch.foot_left.swinging  ? ch.foot_left.swing_target.x  : ch.foot_left.pos.x;
    const double eff_rx = ch.foot_right.swinging ? ch.foot_right.swing_target.x : ch.foot_right.pos.x;

    // Launch a swing step: unpin step_foot, freeze stance_foot.
    // Returns false if step_foot is already swinging (no-op).
    auto launchStep = [&](bool step_left, bool corrective_followthrough) -> bool {
        FootState& step_foot   = step_left ? ch.foot_left  : ch.foot_right;
        FootState& stance_foot = step_left ? ch.foot_right : ch.foot_left;
        if (step_foot.swinging) return false;

        const double tx = computeStepLandingX(m_config.walk, ch, stance_foot,
                                              pelvis, xi, L, reach_radius, ref_slope);
        beginSwingStep(step_foot, stance_foot, ch, tx, m_terrain,
                       corrective_followthrough,
                       L, m_config.step, m_config.walk,
                       std::abs(cm.velocity.x), max_spd);
        return true;
    };

    // ── Airborne foot management ─────────────────────────────────────────────
    // When airborne: cancel any swing in progress, unpin feet so clampToCircle
    // drags them naturally. On landing (airborne→grounded), re-bootstrap so feet
    // start from a sensible ground position instead of stale mid-air pinned_pos.
    if (airborne_final) {
        releaseFeetAirborne(ch);
    } else if (airborne_pre && airborne_final != airborne_pre) {
        // Just landed — re-bootstrap feet at terrain under pelvis.
        // Clamp each foot x so that its terrain point stays inside the 2L disk,
        // even on steep slopes where the naive half-offset would violate it.
        bootstrapFeetOnLanding(ch, m_config.standing, m_terrain,
                               pelvis, reach_radius, L);
    }

    const bool any_swinging = ch.foot_left.swinging || ch.foot_right.swinging;

    if (ch.feet_initialized && ch.step_cooldown <= 0.0 && !any_swinging && !airborne_final) {
        const StepTriggerEval trigger_eval =
            evaluateStepTriggers(ch, eff_lx, eff_rx, xi, cm.velocity.x,
                                 m_config.walk.eps_step, pelvis, L,
                                 m_config.walk.d_rear_max);

        if (trigger_eval.xcom_trigger || trigger_eval.rear_trigger) {
            const bool no_forward_input = std::abs(input_dir) <= 0.01;
            const bool corrective_followthrough =
                no_forward_input && trigger_eval.rear_trigger && !trigger_eval.xcom_trigger;
            launchStep(trigger_eval.step_left_xcom, corrective_followthrough);
            SIM_LOG("Step trigger: %s  d_rear=%.2fL  xcom=%d  rear=%d\n",
                    trigger_eval.step_left_xcom ? "LEFT" : "RIGHT",
                    trigger_eval.d_rear / L,
                    static_cast<int>(trigger_eval.xcom_trigger),
                    static_cast<int>(trigger_eval.rear_trigger));
        }
    }

    // Snapshot ground contact before constraints — used to detect lift-off below.
    const bool was_grounded_L = ch.foot_left.on_ground;
    const bool was_grounded_R = ch.foot_right.on_ground;

    // Constraint order matters: terrain first, then reach clamp, then terrain
    // again to remove tiny post-clamp gaps on slopes.
    applyFootConstraints(ch, m_terrain, pelvis, reach_radius);

    // ── Secondary trigger: immediate recovery when a planted foot lifts off ──
    // If a foot loses terrain contact before the geometric trigger fires, start
    // a corrective step immediately with that same foot, provided the other foot
    // still supports the body.
    tryRecoveryStepOnLiftOff(ch, airborne_final, any_swinging,
                             was_grounded_L, was_grounded_R,
                             input_dir, launchStep);

    // ── Unpin foot that just lost ground contact ─────────────────────────────
    // A pinned foot whose terrain contact disappears (slope change, step edge)
    // must be freed so the step planner can replace it. Only while grounded —
    // the airborne block above already handles the fully-airborne case.
    unpinLiftedFeet(ch, airborne_final, was_grounded_L, was_grounded_R);

    // ── Cache XCoM for renderer ──────────────────────────────────────────────
    cacheXCoMState(m_state, ch, cm, m_config.walk, eff_lx, eff_rx, xi, L);

    // ── Debug ────────────────────────────────────────────────────────────────
    writeDebugState(ch, airborne_final, y_tgt, ground_ref, ref_slope, h_ip,
                    speed_drop, slope_drop, m_config.walk.cm_height_offset);

    // ── Character state (facing, lean, spine, knees) ─────────────────────────
    updateCharacterState(ch, cm, m_config.character, m_config.reconstruction,
                         !airborne_final, dt, ref_slope);
    UpperBodyControl upper_body;
    upper_body.input_dir = input_dir;
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
                         upper_body,
                         dt);
}
