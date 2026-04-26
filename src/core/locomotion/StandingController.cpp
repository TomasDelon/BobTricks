#include "core/locomotion/StandingController.h"

#include <cmath>
#include <algorithm>

static double distance2D(Vec2 a, Vec2 b)
{
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

std::optional<double> computeStandingCMTarget(const SupportState&   support,
                                              const CharacterConfig& char_cfg)
{
    const double L      = char_cfg.body_height_m / 5.0;
    const double r      = 2.0 * L;   // full leg reach — consistent with computeNominalY
    const double half_d = support.width() * 0.5;

    // Standing geometry is impossible when foot separation exceeds full leg reach.
    // Returning nullopt forces the caller to change regime rather than use a
    // silently-degraded target that pushes the CM to the ground.
    if (half_d >= r) return std::nullopt;

    const double h_pelvis = std::sqrt(r * r - half_d * half_d);
    return support.ground_center() + h_pelvis + char_cfg.cm_pelvis_ratio * L;
}

StandingDiag diagStanding(const CMState&        cm,
                           double                xcom,
                           const Vec2&           pelvis,
                           const SupportState&   support,
                           const FootState&      foot_L,
                           const FootState&      foot_R,
                           const CharacterConfig& char_cfg,
                           const StandingConfig&  stand_cfg)
{
    StandingDiag diag;

    const double L         = char_cfg.body_height_m / 5.0;
    const double max_reach = 2.0 * L;   // full leg reach — matches computeStandingCMTarget

    diag.d = support.width();

    // Criterion 1: both feet planted
    diag.c1 = support.both_planted();

    // Criterion 2: foot separation in [d_min, d_max] (×L).
    // A tolerance of 0.03L on the upper bound matches the geometric trigger's dead-band
    // (shouldStep uses ±0.03L) so there is no gap where c2 fails but no step fires.
    static constexpr double EPS_C2 = 0.03;
    diag.c2 = (diag.d >= stand_cfg.d_min * L)
           && (diag.d <= stand_cfg.d_max * L + EPS_C2 * L);

    // Criterion 3: extrapolated CoM (xcom = cm.x + vx/omega0) inside support.
    // Using xcom rather than cm.position.x avoids false negatives when the CM
    // is momentarily displaced but the extrapolated CoM is still in support.
    diag.c3 = (xcom >= support.x_left) && (xcom <= support.x_right);

    // Criterion 4: reach of each leg = |pelvis - foot| <= 2L.
    // With the geometry-corrected nominal height (pelvis at h_pelvis_max, not 2L),
    // legs are fully extended but not over-extended at d_pref separation.
    diag.reach_L = distance2D(pelvis, foot_L.pos);
    diag.reach_R = distance2D(pelvis, foot_R.pos);
    diag.c4 = (diag.reach_L <= max_reach) && (diag.reach_R <= max_reach);

    // Criterion 5: horizontal speed below eps_v
    diag.c5 = (std::abs(cm.velocity.x) <= stand_cfg.eps_v);

    return diag;
}
