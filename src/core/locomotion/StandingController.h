#pragma once

#include "core/character/SupportState.h"
#include "core/character/CMState.h"
#include "core/character/FootState.h"
#include "config/AppConfig.h"

#include <optional>

// Returns the y-coordinate that the CM should converge to in standing pose,
// or nullopt if the foot separation makes standing geometry impossible (d/2 >= 2L).
//
// Geometry (ROADMAP §2.1):
//   L        = body_height_m / 5
//   r        = 2*L                   (full leg reach — same basis as computeNominalY)
//   d        = support.width()
//   h_pelvis = sqrt(r² - (d/2)²)
//   y_CM     = ground_center + h_pelvis + cm_pelvis_ratio * L
//
// Returns nullopt when d/2 >= 2L — physically impossible foot separation;
// caller must handle this as an invalid-regime state, NOT silently use a degraded value.
std::optional<double> computeStandingCMTarget(const SupportState&   support,
                                              const CharacterConfig& char_cfg);

// Per-criterion diagnostics for the 5 validity criteria (ROADMAP §2.3).
// Used by DebugUI and Application logging; individual fields show which
// criterion failed when standing_valid = false.
struct StandingDiag {
    bool   c1      = false;  // both feet planted
    bool   c2      = false;  // d in [d_min*L, d_max*L]
    bool   c3      = false;  // xcom inside support polygon [x_left, x_right]
    bool   c4      = false;  // reach_L <= 2L  &&  reach_R <= 2L
    bool   c5      = false;  // |vx| <= eps_v
    double reach_L = 0.0;    // |pelvis - foot_L|  (m)
    double reach_R = 0.0;    // |pelvis - foot_R|  (m)
    double d       = 0.0;    // support width       (m)

    bool valid() const { return c1 && c2 && c3 && c4 && c5; }
};

// Evaluates the 5 validity criteria and returns per-criterion diagnostics.
// xcom: extrapolated CoM from BalanceState (balance.xcom), used for c3.
// pelvis: current pelvis position (from CharacterState), used for c4 reach check.
StandingDiag diagStanding(const CMState&        cm,
                          double                xcom,
                          const Vec2&           pelvis,
                          const SupportState&   support,
                          const FootState&      foot_L,
                          const FootState&      foot_R,
                          const CharacterConfig& char_cfg,
                          const StandingConfig&  stand_cfg);
