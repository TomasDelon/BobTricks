#include "core/character/HeadController.h"

void resetHeadState(CharacterState& ch)
{
    ch.head_center = {0.0, 0.0};
    ch.head_radius = 0.0;
}

void updateHeadState(CharacterState&        ch,
                     const CharacterConfig& char_config,
                     const HeadConfig&      head_config)
{
    const double L = char_config.body_height_m / 5.0;

    const Vec2 body_up = normalizeOr(ch.torso_top - ch.torso_center, {0.0, 1.0});

    ch.head_radius = head_config.radius_L * L;
    ch.head_center = ch.torso_top + body_up * (head_config.center_offset_L * L);
}
