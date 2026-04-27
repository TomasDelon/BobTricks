#pragma once

/**
 * @file HeadController.h
 * @brief Contrôleur de tête : position cinématique dérivée du torse.
 */

#include "config/AppConfig.h"
#include "core/character/CharacterState.h"

/**
 * @brief Réinitialise l'état dérivé de la tête à ses valeurs par défaut.
 *
 * @param character État du personnage à réinitialiser (modifié en place).
 */
void resetHeadState(CharacterState& character);

/**
 * @brief Met à jour la tête et l'œil visible à partir du torse reconstruit.
 *
 * Cette mise à jour est purement cinématique : elle place la tête et l'œil
 * visible à partir du torse reconstruit. Elle n'affecte jamais la physique.
 *
 * @param character          État du personnage (modifié en place).
 * @param char_config        Configuration morphologique du personnage.
 * @param head_config        Paramètres cinématiques de la tête.
 */
void updateHeadState(CharacterState&        character,
                     const CharacterConfig& char_config,
                     const HeadConfig&      head_config);
