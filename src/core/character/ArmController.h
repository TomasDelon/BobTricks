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
 */
bool solveTwoBoneArm(Vec2 target,
                     double upper_len,
                     double fore_len,
                     Vec2 shoulder,
                     Vec2 bend_preference,
                     const std::optional<Vec2>& previous_elbow,
                     ArmPose& pose);

/** @brief Met à jour les bras dérivés du personnage. */
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
