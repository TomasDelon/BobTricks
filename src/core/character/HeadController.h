#pragma once

/**
 * @file HeadController.h
 * @brief Contrôleur de tête : inclinaison cinématique et orientation du regard.
 */

#include <optional>

#include "config/AppConfig.h"
#include "core/character/CharacterState.h"
#include "core/math/Vec2.h"

/**
 * @brief Réinitialise l'état dérivé de la tête à ses valeurs par défaut.
 *
 * @param character État du personnage à réinitialiser (modifié en place).
 */
void resetHeadState(CharacterState& character);

/**
 * @brief Met à jour la tête et l'œil visible à partir du torse reconstruit.
 *
 * Cette mise à jour est purement cinématique : elle calcule l'inclinaison de la
 * tête en fonction de la vitesse et de la direction, oriente les yeux vers
 * `gaze_target_world` si présent, et filtre les transitions avec la constante
 * de temps `HeadConfig::tau_tilt`. Elle n'affecte jamais la physique.
 *
 * @param character          État du personnage (modifié en place).
 * @param char_config        Configuration morphologique du personnage.
 * @param head_config        Paramètres cinématiques de la tête.
 * @param gaze_target_world  Point de regard optionnel en coordonnées monde.
 * @param dt                 Pas de temps de simulation (s).
 */
void updateHeadState(CharacterState&            character,
                     const CharacterConfig&     char_config,
                     const HeadConfig&          head_config,
                     const std::optional<Vec2>& gaze_target_world,
                     double                     dt);
