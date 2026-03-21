#pragma once

#include "core/character/FootState.h"
#include "core/character/CMState.h"
#include "core/character/StepPlan.h"
#include "core/terrain/Terrain.h"
#include "core/math/Vec2.h"
#include "config/AppConfig.h"

#include "core/locomotion/StepTriggerType.h"

// ── G1: Trigger ──────────────────────────────────────────────────────────────
// Returns true if the rear foot should swing now.
//
// Two rules (all require !plan.active):
//   Normal    : |v_x| > eps_v  AND  behind_dist > k_trigger * L
//               behind_dist = (pelvis.x - rear_foot.x) * facing
//
//   Emergency : friction-braked CM reach has escaped past the front foot.
//               cm_reach = cm.x + vx / floor_friction  (actual stopping point
//               under floor friction with both feet planted and restoring force).
//               fires regardless of velocity.
//
// cm_reach = cm.x + cm.vx / floor_friction (friction-based stopping distance).
// swing_right is set to indicate which foot should swing (the rear one).
// out_trigger is set to Normal, Emergency, or None.
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
                bool*             out_swing_right = nullptr,
                bool*             out_emergency   = nullptr,
                StepTriggerType*  out_trigger     = nullptr);

// ── G2: Plan ─────────────────────────────────────────────────────────────────
// Computes the landing target for the swing foot and returns an active StepPlan.
// Returns an inactive plan (plan.active == false) if the target interval is empty.
//
// swing_right   : true → foot_R swings; false → foot_L swings.
// emergency     : passed through to the plan for logging; does not change target
//                 logic (the spring adapts CM height to any foot placement).
// pelvis_toeoff : reserved for future terrain-aware reach check; currently unused.
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
                  Vec2             pelvis_toeoff);

// ── G3: Swing trajectory ─────────────────────────────────────────────────────
// Evaluates the swing foot position at sim_time.
// u=0 → takeoff position, u=1 → landing target.
Vec2 evalSwingFoot(const StepPlan&  plan,
                   const FootState& swing_foot,
                   const Terrain&   terrain,
                   double           sim_time);
