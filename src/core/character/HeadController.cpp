#include "core/character/HeadController.h"
#include "core/math/MathConstants.h"

#include <algorithm>
#include <cmath>


void resetHeadState(CharacterState& ch)
{
    ch.head_center = {0.0, 0.0};
    ch.eye_left = {0.0, 0.0};
    ch.eye_right = {0.0, 0.0};
    ch.head_radius = 0.0;
    ch.head_tilt = 0.0;
}

void updateHeadState(CharacterState&            ch,
                     const CharacterConfig&     char_config,
                     const HeadConfig&          head_config,
                     const std::optional<Vec2>& gaze_target_world,
                     double                     dt)
{
    const double L = char_config.body_height_m / 5.0;

    Vec2 body_up = normalizeOr(ch.torso_top - ch.torso_center, {0.0, 1.0});
    Vec2 body_right = {body_up.y, -body_up.x};
    if (body_right.x * ch.facing < 0.0)
        body_right = body_right * -1.0;

    ch.head_radius = head_config.radius_L * L;
    ch.head_center = ch.torso_top + body_up * (head_config.center_offset_L * L);

    Vec2 gaze_dir = body_right;
    if (gaze_target_world.has_value())
        gaze_dir = normalizeOr(*gaze_target_world - ch.head_center, body_right);

    const double raw_tilt = std::atan2(dot(gaze_dir, body_up), dot(gaze_dir, body_right));
    const double max_tilt = head_config.max_tilt_deg * kDegToRad;
    const double clamped_tilt = std::clamp(raw_tilt, -max_tilt, max_tilt);

    const double tau = (head_config.tau_tilt > 0.0) ? head_config.tau_tilt : dt;
    const double alpha = 1.0 - std::exp(-dt / std::max(tau, 1.0e-4));
    ch.head_tilt += (clamped_tilt - ch.head_tilt) * alpha;

    const double ct = std::cos(ch.head_tilt);
    const double st = std::sin(ch.head_tilt);
    const Vec2 head_right = body_right * ct + body_up * st;
    const Vec2 head_up    = body_up * ct - body_right * st;

    const Vec2 visible_eye = ch.head_center
                           + head_right * (0.5 * ch.head_radius)
                           + head_up    * (0.25 * ch.head_radius);
    ch.eye_left  = visible_eye;
    ch.eye_right = visible_eye;
}
