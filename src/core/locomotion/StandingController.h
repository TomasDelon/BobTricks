#pragma once

/**
 * @file StandingController.h
 * @brief Calcul de la hauteur cible du CM et diagnostic du régime debout.
 */

#include "core/character/SupportState.h"
#include "core/character/CMState.h"
#include "core/character/FootState.h"
#include "config/AppConfig.h"

#include <optional>

/**
 * @brief Calcule la hauteur cible du centre de masse en régime debout.
 *
 * Retourne `nullopt` si le polygone de support est dégénéré (aucun pied planté).
 *
 * @param support  Polygone de support 1D courant.
 * @param char_cfg Configuration morphologique du personnage.
 * @return Hauteur cible du CM en mètres, ou `nullopt`.
 */
std::optional<double> computeStandingCMTarget(const SupportState&   support,
                                              const CharacterConfig& char_cfg);

/**
 * @brief Détails des cinq critères de validité du régime debout.
 *
 * Le régime debout est valide uniquement si les cinq critères sont satisfaits
 * simultanément (`valid()` retourne vrai).
 */
struct StandingDiag {
    bool   c1      = false; ///< C1 : les deux pieds sont au sol.
    bool   c2      = false; ///< C2 : `d ∈ [d_min·L, d_max·L]` (écartement valide).
    bool   c3      = false; ///< C3 : XCoM est dans le polygone de support `[x_gauche, x_droite]`.
    bool   c4      = false; ///< C4 : portée des deux jambes `<= 2L`.
    bool   c5      = false; ///< C5 : `|vx| <= eps_v` (quasi-immobile).
    double reach_L = 0.0;   ///< Distance bassin–pied gauche (m).
    double reach_R = 0.0;   ///< Distance bassin–pied droit (m).
    double d       = 0.0;   ///< Largeur du polygone de support (m).

    /** @brief Vrai si les cinq critères sont satisfaits. */
    bool valid() const { return c1 && c2 && c3 && c4 && c5; }
};

/**
 * @brief Évalue les cinq critères de validité du régime debout.
 *
 * @param cm        État cinématique courant du CM.
 * @param xcom      Centre de masse extrapolé ξ (m).
 * @param pelvis    Position courante du bassin (m).
 * @param support   Polygone de support 1D.
 * @param foot_L    État du pied gauche.
 * @param foot_R    État du pied droit.
 * @param char_cfg  Configuration morphologique.
 * @param stand_cfg Paramètres géométriques du régime debout.
 * @return Structure `StandingDiag` remplie.
 */
StandingDiag diagStanding(const CMState&        cm,
                          double                xcom,
                          const Vec2&           pelvis,
                          const SupportState&   support,
                          const FootState&      foot_L,
                          const FootState&      foot_R,
                          const CharacterConfig& char_cfg,
                          const StandingConfig&  stand_cfg);
