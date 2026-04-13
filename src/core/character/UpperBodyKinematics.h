#pragma once

#include "config/AppConfig.h"
#include "core/character/ArmController.h"
#include "core/character/HeadController.h"
#include "core/character/UpperBodyTypes.h"

/**
 * @brief Coordonne la mise à jour cinématique du haut du corps.
 *
 * Cette fonction regroupe tête et bras en un seul point d'entrée sans modifier
 * l'état physique autoritatif.
 */
inline void updateUpperBodyState(CharacterState&            character,
                                 const CMState&             cm,
                                 const CharacterConfig&     char_config,
                                 const PhysicsConfig&       physics_config,
                                 const WalkConfig&          walk_config,
                                 const ArmConfig&           arm_config,
                                 const HeadConfig&          head_config,
                                 const UpperBodyControl&    control,
                                 double                     dt)
{
    updateArmState(character, cm, char_config, physics_config, walk_config, arm_config,
                   control.input_dir,
                   control.targets.left_hand_target,
                   control.targets.right_hand_target,
                   dt);
    updateHeadState(character, char_config, head_config, control.targets.gaze_target_world, dt);
}
