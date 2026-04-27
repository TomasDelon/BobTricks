#pragma once

/**
 * @file UpperBodyTypes.h
 * @brief Types d'entrée pour la cinématique du haut du corps.
 *
 * Ces structures servent d'interface entre `InputController` et
 * `UpperBodyKinematics`, isolant la logique de contrôle des données de pose.
 */

#include <optional>

#include "core/math/Vec2.h"

/**
 * @brief Cibles optionnelles injectées dans la cinématique du haut du corps.
 *
 * Chaque cible est optionnelle : si absente, le contrôleur correspondant
 * utilise son comportement procédural par défaut.
 */
struct UpperBodyTargets {
    std::optional<Vec2> left_hand_target;   ///< Cible de main gauche en coordonnées monde (m).
    std::optional<Vec2> right_hand_target;  ///< Cible de main droite en coordonnées monde (m).
};

/**
 * @brief Entrées agrégées pour la mise à jour du haut du corps.
 *
 * Transmis à `updateUpperBodyState()` une fois par pas de simulation.
 */
struct UpperBodyControl {
    double           input_dir = 0.0; ///< Direction d'entrée normalisée (`-1` gauche, `+1` droite, `0` neutre).
    UpperBodyTargets targets;          ///< Cibles optionnelles de membres.
};
