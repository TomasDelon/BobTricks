#include "core/character/UpperBodyKinematics.h"

void updateUpperBodyState(CharacterState&         character,
                          const CMState&          cm,
                          const CharacterConfig&  char_config,
                          const PhysicsConfig&    physics_config,
                          const WalkConfig&       walk_config,
                          const ArmConfig&        arm_config,
                          const HeadConfig&       head_config,
                          const UpperBodyControl& control,
                          double                  dt)
{
    updateArmState(character, cm, char_config, physics_config, walk_config, arm_config,
                   control.input_dir,
                   control.targets.left_hand_target,
                   control.targets.right_hand_target,
                   dt);
    updateHeadState(character, char_config, head_config, control.targets.gaze_target_world, dt);
}
