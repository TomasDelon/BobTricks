#pragma once

/**
 * @file UpperBodyKinematics.h
 * @brief Point d'entrée unique pour la mise à jour cinématique du haut du corps.
 *
 * Ce fichier regroupe `ArmController` et `HeadController` derrière une seule
 * fonction, garantissant un ordre d'exécution cohérent et simplifiant
 * l'appel depuis `SimulationCore`.
 */

#include "config/AppConfig.h"
#include "core/character/ArmController.h"
#include "core/character/HeadController.h"
#include "core/character/UpperBodyTypes.h"

/**
 * @brief Coordonne la mise à jour cinématique du haut du corps.
 *
 * Appelle successivement `updateArmState` puis `updateHeadState`. Cette
 * fonction ne modifie pas l'état physique autoritatif (CM, pieds).
 *
 * @param character      État du personnage (modifié en place).
 * @param cm             État courant du centre de masse.
 * @param char_config    Configuration morphologique.
 * @param physics_config Paramètres physiques (nécessaires aux bras).
 * @param walk_config    Paramètres de marche (vitesse de phase, arcs).
 * @param arm_config     Paramètres cinématiques des bras.
 * @param head_config    Paramètres cinématiques de la tête.
 * @param control        Entrées agrégées (direction et cibles de membres).
 * @param dt             Pas de temps de simulation (s).
 */
void updateUpperBodyState(CharacterState&         character,
                          const CMState&          cm,
                          const CharacterConfig&  char_config,
                          const PhysicsConfig&    physics_config,
                          const WalkConfig&       walk_config,
                          const ArmConfig&        arm_config,
                          const HeadConfig&       head_config,
                          const UpperBodyControl& control,
                          double                  dt);
