#pragma once

/**
 * @file SimState.h
 * @brief Instantané complet de l'état de simulation et conditions initiales.
 */

#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "core/math/Vec2.h"

#include <cstdint>

/**
 * @brief Événements ponctuels émis par le noyau de simulation lors d'un pas.
 *
 * Ces drapeaux sont positionnés par `SimulationCore::step()` et consommés
 * par `AudioSystem` et `EffectsSystem` pour déclencher les effets sonores et
 * visuels correspondants. Ils sont réinitialisés à chaque pas.
 */
struct SimEvents {
    bool left_touchdown    = false; ///< Pied gauche vient de toucher le sol.
    bool right_touchdown   = false; ///< Pied droit vient de toucher le sol.
    bool landed_from_jump  = false; ///< Atterrissage après un saut.
    bool left_slide_active  = false; ///< Pied gauche glisse sur le terrain.
    bool right_slide_active = false; ///< Pied droit glisse sur le terrain.
};

/**
 * @brief Instantané complet de l'état de simulation à un instant donné.
 *
 * Cette structure est utilisée pour l'affichage, la télémétrie et le
 * mécanisme de step-back. Elle contient le CM autoritatif, l'état dérivé
 * du personnage, les événements de contact et le XCoM mis en cache.
 */
struct SimState {
    CMState        cm;        ///< État cinématique autoritatif du centre de masse.
    CharacterState character; ///< Pose et état des membres dérivés du CM.
    SimEvents      events;    ///< Événements ponctuels de contact et de glissement.
    double         sim_time = 0.0; ///< Temps de simulation cumulé (s).

    /** @brief Centre de masse extrapolé `ξ = x_cm + α·v/ω₀` mis en cache (m).
     *  Calculé dans `SimulationCore::step` pour éviter la recomputation dans les renderers. */
    double xi            = 0.0;
    double xi_target_x   = 0.0; ///< Cible d'atterrissage : `ξ + facing × margin × L` (m).
    bool   xi_trigger    = false; ///< Vrai si ξ a dépassé le pied avant (pas requis).
};

/**
 * @brief Conditions initiales utilisées par `SimulationCore::reset()` et les scénarios headless.
 */
struct ScenarioInit {
    Vec2     cm_pos       = {0.0, 0.0}; ///< Position initiale du CM (m).
    Vec2     cm_vel       = {0.0, 0.0}; ///< Vitesse initiale du CM (m/s).
    uint32_t terrain_seed = 0;           ///< Graine du terrain (0 = conserver la graine courante).
};
