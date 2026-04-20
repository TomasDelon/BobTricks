#pragma once

/**
 * @file FootState.h
 * @brief État complet d'un pied : contact au sol, ancrage et arc de swing.
 */

#include "core/math/Vec2.h"

/**
 * @brief État complet d'un pied : contact au sol, ancrage manuel et trajectoire de swing.
 *
 * Un pied peut se trouver dans l'un des états mutuellement exclusifs suivants :
 * - **ancré** (`pinned = true`) : le pied est verrouillé à `pinned_pos` ;
 * - **en swing** (`swinging = true`) : le pied suit l'arc parabolique de
 *   `swing_start` vers `swing_target` au fur et à mesure que `swing_t` progresse ;
 * - **au sol** (`on_ground = true`) : le pied suit le terrain de manière passive.
 */
struct FootState {
    Vec2 pos           = {0.0, 0.0}; ///< Position monde courante du pied (m).
    bool on_ground     = false;       ///< Vrai si le pied touche le terrain.
    Vec2 ground_normal = {0.0, 1.0};  ///< Normale sortante du terrain au point de contact.
    bool airborne      = false;       ///< Vrai quand le pied est volontairement hors des contraintes terrain (saut).

    bool pinned        = false;       ///< Le pied est verrouillé à `pinned_pos` (clic droit).
    Vec2 pinned_pos    = {0.0, 0.0};  ///< Position d'ancrage en coordonnées monde (m).
    Vec2 pinned_normal = {0.0, 1.0};  ///< Normale du terrain gelée au moment de l'ancrage.

    bool   swinging          = false;      ///< Vrai pendant l'arc de swing.
    Vec2   swing_start       = {0.0, 0.0}; ///< Position de départ du swing.
    Vec2   swing_target      = {0.0, 0.0}; ///< Position cible d'atterrissage.
    double swing_t           = 0.0;        ///< Progression du swing dans `[0, 1]`.

    double swing_speed_scale = 1.0; ///< Facteur de ralentissement sur les pas raides `[0,1]`.
    double swing_h_clear     = 0.0; ///< Hauteur de dégagement parabolique pour ce pas (m).
};
