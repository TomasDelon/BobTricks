#pragma once

/**
 * @file SupportState.h
 * @brief Polygone de support sagittal réduit à deux appuis ponctuels.
 *
 * @note Simplification V4 : chaque pied contribue un unique point `pos.x`,
 * non un intervalle talon/pointe. La marge de stabilité est calculée par
 * rapport à ces deux points et non à un rectangle de sustentation réel.
 * Quand des segments de pied seront ajoutés, remplacer `x_left`/`x_right` par
 * `x_heel_left`/`x_toe_right` (extrémités du polygone de support).
 */

/**
 * @brief Intervalle de support 1D (sagittal) dérivé des pieds posés au sol.
 *
 * Les valeurs `x_left`/`x_right` sont dérivées de `FootState::pos.x` des pieds
 * plantés — jamais des cibles prévues.
 */
struct SupportState {
    double x_left  = 0.0; ///< Position X du pied gauche planté (m).
    double x_right = 0.0; ///< Position X du pied droit planté (m).

    double y_left  = 0.0; ///< Hauteur du terrain sous le pied gauche (m) — pour l'IK et le setpoint CM.
    double y_right = 0.0; ///< Hauteur du terrain sous le pied droit (m).

    bool left_planted  = false; ///< Vrai si le pied gauche est planté au sol.
    bool right_planted = false; ///< Vrai si le pied droit est planté au sol.

    /** @brief Centre horizontal de l'intervalle de support (m). */
    double center() const;
    /** @brief Largeur de l'intervalle de support (m). */
    double width() const;
    /** @brief Hauteur de terrain moyenne entre les deux appuis (m). */
    double ground_center() const;
    /** @brief Vrai si les deux pieds sont simultanément au sol. */
    bool both_planted() const;
};
