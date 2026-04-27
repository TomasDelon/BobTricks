#pragma once

/**
 * @file InputFrame.h
 * @brief Structure d'entrée consommée par `SimulationCore` à chaque pas fixe.
 */

#include <optional>
#include "core/math/Vec2.h"

/**
 * @brief Entrées produites par `InputController` et consommées par `SimulationCore`.
 *
 * Cette structure est une interface sans état entre la couche SDL et le noyau
 * de simulation. Elle est construite à chaque frame par `InputController` et
 * passée à `SimulationCore::step()`.
 */
struct InputFrame {
    bool key_left  = false; ///< Touche ← maintenue.
    bool key_right = false; ///< Touche → maintenue.
    bool key_run   = false; ///< Modificateur Shift (course).
    bool jump      = false; ///< Espace pressé ce tick (impulsion de saut).

    std::optional<Vec2> set_velocity;       ///< Glisser clic-droit : téléporte la vitesse du CM au début du pas.

    bool foot_left_drag  = false;          ///< Clic-gauche actif sur le pied gauche.
    Vec2 foot_left_pos   = {0.0, 0.0};     ///< Cible monde du pied gauche (m) quand `foot_left_drag`.
    bool foot_right_drag = false;          ///< Clic-gauche actif sur le pied droit.
    Vec2 foot_right_pos  = {0.0, 0.0};     ///< Cible monde du pied droit (m) quand `foot_right_drag`.

    bool hand_left_drag  = false;          ///< Clic-gauche actif sur la main gauche.
    Vec2 hand_left_pos   = {0.0, 0.0};     ///< Cible IK monde de la main gauche (m).
    bool hand_right_drag = false;          ///< Clic-gauche actif sur la main droite.
    Vec2 hand_right_pos  = {0.0, 0.0};     ///< Cible IK monde de la main droite (m).
};
