#include "core/locomotion/StepPlanner.h"

#include <cmath>
#include <algorithm>
#include <cstdio>   // fprintf

#include "core/simulation/SimVerbosity.h"

// ── helpers ──────────────────────────────────────────────────────────────────

static double quintic(double u)
{
    // S5(u) = 10u³ - 15u⁴ + 6u⁵  (smoothstep, zero velocity at endpoints)
    return u * u * u * (10.0 + u * (-15.0 + 6.0 * u));
}

static double clampd(double v, double lo, double hi)
{
    return std::max(lo, std::min(hi, v));
}

// ── G1 ───────────────────────────────────────────────────────────────────────

bool shouldStep(const Vec2&       pelvis,
                const FootState&  foot_L,
                const FootState&  foot_R,
                const CMState&    cm,
                double            cm_reach,
                const StepPlan&   plan,
                const StandingConfig& stand_cfg,
                const WalkConfig&     walk_cfg,
                double            L,
                double            facing,
                bool*             out_swing_right,
                bool*             out_emergency,
                StepTriggerType*  out_trigger)
{
    if (out_swing_right) *out_swing_right = false;
    if (out_emergency)   *out_emergency   = false;
    if (out_trigger)     *out_trigger     = StepTriggerType::None;

    // Never interrupt an in-flight step.
    if (plan.active) return false;

    // Identify rear foot: the one further behind in the walking direction.
    // s = facing * x  →  smaller s = more behind.
    const double s_L = facing * foot_L.pos.x;
    const double s_R = facing * foot_R.pos.x;
    const bool   swing_right = (s_R <= s_L);   // R is rear (or tied) → swing R

    const FootState& rear  = swing_right ? foot_R : foot_L;
    const FootState& front = swing_right ? foot_L : foot_R;

    if (out_swing_right) *out_swing_right = swing_right;

    // behind_dist: how far the rear foot is behind the pelvis (always >= 0).
    const double behind_dist = (pelvis.x - rear.pos.x) * facing;

    // ── Rule 1 — Normal trigger (requires movement) ───────────────────────
    const bool normal = (std::abs(cm.velocity.x) > stand_cfg.eps_v)
                     && (behind_dist > walk_cfg.k_trigger * L);

    if (normal) {
        if (out_trigger) *out_trigger = StepTriggerType::Normal;
        if (g_sim_verbose)
            fprintf(stderr, "[StepPlanner] trigger=Normal  swing=%s  behind=%.3f  threshold=%.3f\n",
                    swing_right ? "R" : "L", behind_dist, walk_cfg.k_trigger * L);
        return true;
    }

    // ── Rule 2 — Emergency trigger (friction-braked reach past the front foot)
    // cm_reach = cm.x + vx / floor_friction: where the CM actually stops under
    // floor friction with both feet planted and the restoring force active.
    // This is shorter than the IP capture point (μ=4 > ω₀≈3.18), so it only
    // fires when truly necessary — no spurious steps during deceleration.
    const double reach_ahead = (cm_reach - front.pos.x) * facing;
    if (reach_ahead > 0.0) {
        if (out_emergency) *out_emergency = true;
        if (out_trigger)   *out_trigger   = StepTriggerType::Emergency;
        if (g_sim_verbose)
            fprintf(stderr, "[StepPlanner] trigger=Emergency  swing=%s  reach_ahead=%.3f\n",
                    swing_right ? "R" : "L", reach_ahead);
        return true;
    }

    return false;
}

// ── G2 ───────────────────────────────────────────────────────────────────────

StepPlan planStep(bool             swing_right,
                  bool             emergency,
                  const FootState& foot_swing,
                  const FootState& foot_stance,
                  const CMState&   cm,
                  const StepConfig&     step_cfg,
                  const StandingConfig& stand_cfg,
                  const WalkConfig&     walk_cfg,
                  const Terrain&   terrain,
                  double           sim_time,
                  double           L,
                  double           facing,
                  Vec2             pelvis_toeoff)
{
    (void)pelvis_toeoff;  // reserved for future terrain-aware reach check
    (void)stand_cfg;      // spring adapts CM height to any foot placement

    // ── Desired landing position ─────────────────────────────────────────────
    // Target: CM + k_step*L ahead in facing dir + velocity anticipation.
    // CM-relative (not stance-relative) avoids leg-reach infeasibility.
    const double v_x       = cm.velocity.x;
    const double x_desired = cm.position.x
                           + facing * (walk_cfg.k_step * L
                                     + std::abs(v_x) * walk_cfg.T_ant);

    // ── Feasible interval ────────────────────────────────────────────────────
    // Foot must land ahead of stance foot (no leg crossing) and
    // within d_max_correction*L of it (no overstride).
    // No I_reach: the spring adapts the CM height, rigid reach is irrelevant.
    const double d_max_corr = step_cfg.d_max_correction * L;
    const double epsilon     = 0.01;
    double f_lo, f_hi;
    if (facing > 0.0) {
        f_lo = foot_stance.pos.x + epsilon;
        f_hi = foot_stance.pos.x + d_max_corr;
    } else {
        f_lo = foot_stance.pos.x - d_max_corr;
        f_hi = foot_stance.pos.x - epsilon;
    }

    if (f_lo > f_hi) {
        if (g_sim_verbose)
            fprintf(stderr, "[StepPlanner] infeasible: I_width empty\n");
        return {};
    }

    const double x_land = clampd(x_desired, f_lo, f_hi);

    // ── Swing duration ────────────────────────────────────────────────────────
    const double dist    = std::abs(x_land - foot_swing.pos.x);
    const double T_swing = clampd(step_cfg.T_min + step_cfg.dist_coeff * dist / L,
                                  step_cfg.T_min, step_cfg.T_max);
    const double h_clear = step_cfg.h_clear_ratio * L;

    if (g_sim_verbose)
    fprintf(stderr, "[StepPlanner] planStep: swing=%s  emergency=%d  x_land=%.3f  T=%.3f\n",
            swing_right ? "R" : "L", static_cast<int>(emergency), x_land, T_swing);

    return {
        true,
        swing_right,
        foot_swing.pos,
        foot_swing.ground_y,
        { x_land, terrain.height_at(x_land) },
        sim_time,
        T_swing,
        h_clear
    };
}

// ── G3 ───────────────────────────────────────────────────────────────────────

Vec2 evalSwingFoot(const StepPlan&  plan,
                   const FootState& /*swing_foot*/,
                   const Terrain&   terrain,
                   double           sim_time)
{
    const double u_raw = (sim_time - plan.t_start) / plan.duration;
    const double u     = clampd(u_raw, 0.0, 1.0);

    const double x_takeoff = plan.takeoff_pos.x;
    const double x_land    = plan.land_target.x;
    const double x         = x_takeoff + (x_land - x_takeoff) * quintic(u);

    const double y_takeoff = plan.takeoff_ground;
    const double y_landing = terrain.height_at(x_land);
    const double y         = y_takeoff + (y_landing - y_takeoff) * u
                           + plan.clearance * std::sin(M_PI * u);

    return { x, y };
}
