#pragma once

#include "core/character/SupportState.h"
#include "core/character/CMState.h"
#include "core/character/FootState.h"
#include "config/AppConfig.h"

#include <optional>

/**
 * @brief Calcule la hauteur cible du centre de masse en régime debout.
 */
std::optional<double> computeStandingCMTarget(const SupportState&   support,
                                              const CharacterConfig& char_cfg);

/** @brief Détails des critères de validité du régime debout. */
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

/** @brief Évalue les critères de validité du régime debout. */
StandingDiag diagStanding(const CMState&        cm,
                          double                xcom,
                          const Vec2&           pelvis,
                          const SupportState&   support,
                          const FootState&      foot_L,
                          const FootState&      foot_R,
                          const CharacterConfig& char_cfg,
                          const StandingConfig&  stand_cfg);
