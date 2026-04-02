#pragma once

#include <optional>

#include "config/AppConfig.h"
#include "core/character/CharacterState.h"
#include "core/math/Vec2.h"

/**
 * @brief Réinitialise l'état dérivé de la tête.
 * @param character État du personnage à réinitialiser.
 */
void resetHeadState(CharacterState& character);

/**
 * @brief Met à jour la tête et l'œil visible à partir du torse reconstruit.
 *
 * Cette mise à jour est purement cinématique et n'affecte jamais la physique.
 * @param character         État du personnage à compléter.
 * @param char_config       Paramètres morphologiques globaux.
 * @param head_config       Paramètres de regard et de tilt de la tête.
 * @param gaze_target_world Cible de regard optionnelle en coordonnées monde.
 * @param dt                Pas fixe courant.
 */
void updateHeadState(CharacterState&            character,
                     const CharacterConfig&     char_config,
                     const HeadConfig&          head_config,
                     const std::optional<Vec2>& gaze_target_world,
                     double                     dt);
