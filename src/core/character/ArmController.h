#pragma once

/**
 * @file ArmController.h
 * @brief Contrôleur de bras : IK analytique à deux segments et oscillation contralatérale.
 */

#include "config/AppConfig.h"
#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include <optional>

/**
 * @brief Pose cinématique d'un bras à deux segments (épaule–coude–main).
 */
struct ArmPose {
    Vec2 shoulder = {0.0, 0.0}; ///< Position de l'épaule (m).
    Vec2 elbow    = {0.0, 0.0}; ///< Position du coude (m).
    Vec2 hand     = {0.0, 0.0}; ///< Position de la main (m).
    bool reached  = false;       ///< Vrai si la cible est accessible sans clampement.
};

/**
 * @brief Résout l'IK analytique d'un bras à deux segments.
 *
 * La préférence de pliage (`bend_preference`) oriente le coude vers la solution
 * désirée parmi les deux branches géométriques. Le coude précédent (`previous_elbow`)
 * stabilise la sélection de branche sur plusieurs frames consécutives.
 *
 * @param target          Cible de la main en coordonnées monde (m).
 * @param upper_len       Longueur du segment épaule–coude (m).
 * @param fore_len        Longueur du segment coude–main (m).
 * @param shoulder        Position de l'épaule (m).
 * @param bend_preference Direction préférée du coude (vecteur non normalisé).
 * @param previous_elbow  Position du coude au pas précédent (peut être absent).
 * @param pose            Résultat de la pose IK (sortie).
 * @return Vrai si la cible est atteignable.
 */
bool solveTwoBoneArm(Vec2 target,
                     double upper_len,
                     double fore_len,
                     Vec2 shoulder,
                     Vec2 bend_preference,
                     const std::optional<Vec2>& previous_elbow,
                     ArmPose& pose);

/**
 * @brief Met à jour l'état des bras du personnage pour ce pas de simulation.
 *
 * Cette fonction gère l'oscillation contralatérale couplée au cycle de pas,
 * le mélange marche/course, le suivi de cibles manuelles et le filtre de phase.
 */
void updateArmState(CharacterState& ch,
                    const CMState& cm,
                    const CharacterConfig& char_config,
                    const PhysicsConfig& physics_config,
                    const WalkConfig& walk_config,
                    const ArmConfig& arm_config,
                    double input_dir,
                    const std::optional<Vec2>& left_hand_target,
                    const std::optional<Vec2>& right_hand_target,
                    double dt);
