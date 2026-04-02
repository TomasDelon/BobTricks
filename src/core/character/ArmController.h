#pragma once

#include "config/AppConfig.h"
#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include <optional>

/** @brief Pose cinématique d'un bras à deux segments. */
struct ArmPose {
    Vec2 shoulder = {0.0, 0.0};
    Vec2 elbow    = {0.0, 0.0};
    Vec2 hand     = {0.0, 0.0};
    bool reached  = false;
};

/**
 * @brief Résout l'IK analytique d'un bras à deux segments.
 *
 * La préférence de pliage et, si disponible, le coude précédent servent à
 * stabiliser le choix entre les deux branches géométriques.
 * @param target          Cible monde visée par la main.
 * @param upper_len       Longueur du segment épaule-coude.
 * @param fore_len        Longueur du segment coude-main.
 * @param shoulder        Point d'ancrage du bras sur le torse.
 * @param bend_preference Direction privilégiée pour le pliage du coude.
 * @param previous_elbow  Position précédente du coude pour stabiliser la branche.
 * @param pose            Résultat cinématique rempli par la fonction.
 * @return `true` si la cible a été atteinte sans saturation géométrique.
 */
bool solveTwoBoneArm(Vec2 target,
                     double upper_len,
                     double fore_len,
                     Vec2 shoulder,
                     Vec2 bend_preference,
                     const std::optional<Vec2>& previous_elbow,
                     ArmPose& pose);

/**
 * @brief Met à jour les bras dérivés du personnage.
 *
 * Cette fonction applique la trajectoire de marche, les targets drag/pin et
 * résout ensuite l'IK de chaque bras.
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
