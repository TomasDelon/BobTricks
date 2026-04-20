#pragma once

/**
 * @file CMState.h
 * @brief État cinématique du centre de masse (CM) du personnage.
 */

#include "core/math/Vec2.h"

/**
 * @brief État cinématique courant du centre de masse.
 *
 * Le CM est le point de référence autoritatif de la simulation de locomotion.
 * Toutes les autres positions du corps en sont dérivées.
 */
struct CMState {
    Vec2 position     = {0.0, 0.0}; ///< Position monde du CM (m).
    Vec2 velocity     = {0.0, 0.0}; ///< Vitesse du CM (m/s).
    Vec2 acceleration = {0.0, 0.0}; ///< Accélération nette ce pas (m/s²) — utilisée pour l'affichage.
};
