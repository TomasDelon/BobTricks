#pragma once

#include <optional>
#include "core/math/Vec2.h"

/**
 * @brief Entrées consommées par `SimulationCore` pendant un pas fixe.
 */
struct InputFrame {
    /** @brief Demande d'accélération horizontale vers la gauche. */
    bool key_left  = false;
    /** @brief Demande d'accélération horizontale vers la droite. */
    bool key_right = false;
    /** @brief Demande de saut pour le pas fixe courant. */
    bool jump      = false;

    /** @brief Vitesse du CM imposée au début du pas, via drag droit. */
    std::optional<Vec2> set_velocity;

    /** @brief Cible monde optionnelle utilisée par la cinématique de regard. */
    std::optional<Vec2> gaze_target_world;

    /** @brief Active le drag du pied gauche vers une cible monde. */
    bool              foot_left_drag  = false;
    /** @brief Cible monde du drag du pied gauche. */
    Vec2              foot_left_pos   = {0.0, 0.0};
    /** @brief Active le drag du pied droit vers une cible monde. */
    bool              foot_right_drag = false;
    /** @brief Cible monde du drag du pied droit. */
    Vec2              foot_right_pos  = {0.0, 0.0};

    /** @brief Active le drag de la main gauche vers une cible monde. */
    bool              hand_left_drag  = false;
    /** @brief Cible monde du drag de la main gauche. */
    Vec2              hand_left_pos   = {0.0, 0.0};
    /** @brief Active le drag de la main droite vers une cible monde. */
    bool              hand_right_drag = false;
    /** @brief Cible monde du drag de la main droite. */
    Vec2              hand_right_pos  = {0.0, 0.0};
};
