#include "core/character/ArmController.h"
#include "core/math/MathConstants.h"

#include <algorithm>
#include <cmath>

namespace {

Vec2 armCirclePoint(Vec2 center, Vec2 body_right, Vec2 body_up, double radius, double angle_deg)
{
    const double a = angle_deg * kDegToRad;
    return center + body_right * (std::cos(a) * radius)
                  + body_up    * (std::sin(a) * radius);
}

double expDecayFactor(double rate, double dt)
{
    return 1.0 - std::exp(-std::max(0.0, rate) * std::max(0.0, dt));
}

double signedSpeedFromSwingDelta(double delta)
{
    if (std::abs(delta) <= kEpsAngle)
        return 0.0;
    return (delta > 0.0) ? 1.0 : -1.0;
}

double scaleArcAngle(double start_deg, double end_deg, double t, double amp_scale)
{
    const double mid = 0.5 * (start_deg + end_deg);
    const double raw = std::lerp(start_deg, end_deg, t);
    return mid + (raw - mid) * amp_scale;
}

} // namespace

bool solveTwoBoneArm(Vec2 target,
                     double upper_len,
                     double fore_len,
                     Vec2 shoulder,
                     Vec2 bend_preference,
                     const std::optional<Vec2>& previous_elbow,
                     ArmPose& pose)
{
    pose.shoulder = shoulder;

    const Vec2 raw_delta = target - shoulder;
    const double raw_dist = raw_delta.length();
    const double max_reach = upper_len + fore_len;
    const double min_reach = std::abs(upper_len - fore_len);

    bool reached = true;
    Vec2 hand = target;

    if (raw_dist > max_reach) {
        hand = shoulder + normalizeOr(raw_delta, {1.0, 0.0}) * max_reach;
        reached = false;
    } else if (raw_dist < min_reach) {
        hand = shoulder + normalizeOr(raw_delta, {1.0, 0.0}) * min_reach;
        reached = false;
    }

    const Vec2 delta = hand - shoulder;
    const double dist = delta.length();
    const Vec2 dir = normalizeOr(delta, {1.0, 0.0});

    if (dist <= kEpsLength) {
        const Vec2 bend_dir = normalizeOr(bend_preference, {0.0, -1.0});
        pose.elbow = shoulder + bend_dir * upper_len;
        pose.hand = shoulder;
        pose.reached = false;
        return false;
    }

    const double a = (upper_len * upper_len - fore_len * fore_len + dist * dist)
                   / (2.0 * dist);
    const double h_sq = std::max(0.0, upper_len * upper_len - a * a);
    const double h = std::sqrt(h_sq);
    const Vec2 mid = shoulder + dir * a;
    const Vec2 perp = {-dir.y, dir.x};

    Vec2 elbow_a = mid + perp * h;
    Vec2 elbow_b = mid - perp * h;
    const double score_a = dot(elbow_a - shoulder, bend_preference);
    const double score_b = dot(elbow_b - shoulder, bend_preference);
    if (score_b > score_a) {
        std::swap(elbow_a, elbow_b);
    } else if (std::abs(score_a - score_b) <= kEpsAngle && previous_elbow.has_value()) {
        const double da = (elbow_a - *previous_elbow).length();
        const double db = (elbow_b - *previous_elbow).length();
        if (db < da)
            std::swap(elbow_a, elbow_b);
    }

    pose.elbow = elbow_a;
    pose.hand = hand;
    pose.reached = reached;
    return reached;
}

void updateArmState(CharacterState& ch,
                    const CMState& cm,
                    const CharacterConfig& char_config,
                    const PhysicsConfig& physics_config,
                    const WalkConfig& walk_config,
                    const ArmConfig& arm_config,
                    double input_dir,
                    const std::optional<Vec2>& left_hand_target,
                    const std::optional<Vec2>& right_hand_target,
                    double dt)
{
    const double L = char_config.body_height_m / 5.0;
    const Vec2 body_up = normalizeOr(ch.torso_top - ch.torso_center, {0.0, 1.0});
    Vec2 body_right = {body_up.y, -body_up.x};
    if (body_right.x * ch.facing < 0.0)
        body_right = body_right * -1.0;

    const Vec2 shoulder_root = ch.torso_top;

    ch.shoulder_left  = shoulder_root;
    ch.shoulder_right = shoulder_root;

    const bool walking = std::abs(input_dir) > 0.01;
    const bool front_hand_is_left = (ch.facing < 0.0);
    const bool contralateral_front_foot_is_left = !front_hand_is_left;
    const bool left_foot_active = ch.foot_left.swinging && !ch.foot_right.swinging;
    const bool right_foot_active = ch.foot_right.swinging && !ch.foot_left.swinging;
    const bool use_leg_coupled_cycle = left_foot_active || right_foot_active;
    if (!ch.arm_pose_initialized)
        ch.arm_run_blend = std::clamp(ch.run_blend, 0.0, 1.0);
    const double arm_run_tau = std::max(arm_config.run_blend_tau, 1.0e-4);
    const double arm_run_alpha = std::clamp(dt / std::max(arm_run_tau, 1.0e-4), 0.0, 1.0);
    ch.arm_run_blend += (std::clamp(ch.run_blend, 0.0, 1.0) - ch.arm_run_blend) * arm_run_alpha;
    ch.arm_run_blend = std::clamp(ch.arm_run_blend, 0.0, 1.0);
    const double rb = ch.arm_run_blend;

    const double hand_reach_reduction_L =
        std::lerp(arm_config.walk_hand_reach_reduction_L, arm_config.run_hand_reach_reduction_L, rb);
    const double hand_phase_speed_scale =
        std::lerp(arm_config.walk_hand_phase_speed_scale, arm_config.run_hand_phase_speed_scale, rb);
    const double hand_speed_arc_gain =
        std::lerp(arm_config.walk_hand_speed_arc_gain, arm_config.run_hand_speed_arc_gain, rb);
    const double hand_phase_response =
        std::lerp(arm_config.walk_hand_phase_response, arm_config.run_hand_phase_response, rb);
    const double hand_phase_friction =
        std::lerp(arm_config.walk_hand_phase_friction, arm_config.run_hand_phase_friction, rb);
    const double front_hand_start_deg =
        std::lerp(arm_config.walk_front_hand_start_deg, arm_config.run_front_hand_start_deg, rb);
    const double front_hand_end_deg =
        std::lerp(arm_config.walk_front_hand_end_deg, arm_config.run_front_hand_end_deg, rb);
    const double back_hand_start_deg =
        std::lerp(arm_config.walk_back_hand_start_deg, arm_config.run_back_hand_start_deg, rb);
    const double back_hand_end_deg =
        std::lerp(arm_config.walk_back_hand_end_deg, arm_config.run_back_hand_end_deg, rb);

    const double cycle_hz = hand_phase_speed_scale * walk_config.step_speed;
    const double natural_swing_speed = std::max(0.25, cycle_hz);
    if (!ch.arm_pose_initialized) {
        ch.arm_phase = 0.5;
        ch.arm_phase_velocity = 0.0;
    }

    std::optional<double> swing_t_target;
    std::optional<double> swing_t_target_velocity;
    if (use_leg_coupled_cycle) {
        const bool active_swing_foot_is_left = left_foot_active;
        const double raw_leg_t = active_swing_foot_is_left ? ch.foot_left.swing_t
                                                           : ch.foot_right.swing_t;
        const bool direct_cycle = (active_swing_foot_is_left == contralateral_front_foot_is_left);
        const double leg_swing_t = direct_cycle ? std::clamp(raw_leg_t, 0.0, 1.0) : (1.0 - std::clamp(raw_leg_t, 0.0, 1.0));
        swing_t_target = leg_swing_t;
        swing_t_target_velocity = signedSpeedFromSwingDelta(leg_swing_t - ch.arm_phase) * natural_swing_speed;
    }

    if (swing_t_target.has_value()) {
        const double response = expDecayFactor(hand_phase_response, dt);
        ch.arm_phase_velocity += ((*swing_t_target_velocity) - ch.arm_phase_velocity) * response;
        ch.arm_phase += ch.arm_phase_velocity * dt;
        ch.arm_phase += ((*swing_t_target) - ch.arm_phase) * response;
    } else if (walking) {
        const double damping = std::exp(-0.5 * std::max(0.0, hand_phase_friction) * std::max(0.0, dt));
        ch.arm_phase_velocity *= damping;
        ch.arm_phase += ch.arm_phase_velocity * dt;
    } else {
        const double damping = std::exp(-std::max(0.0, hand_phase_friction) * std::max(0.0, dt));
        ch.arm_phase_velocity *= damping;
        ch.arm_phase += ch.arm_phase_velocity * dt;
    }
    ch.arm_phase = std::clamp(ch.arm_phase, 0.0, 1.0);
    if ((ch.arm_phase <= 0.0 && ch.arm_phase_velocity < 0.0)
        || (ch.arm_phase >= 1.0 && ch.arm_phase_velocity > 0.0)) {
        ch.arm_phase_velocity = 0.0;
    }

    const double upper_len = arm_config.upper_arm_L * L;
    const double fore_len = arm_config.fore_arm_L * L;
    const Vec2 elbow_bend_pref = body_right * -1.0 + body_up * -0.15;
    const bool keep_branch_memory = ch.arm_pose_initialized
                                 && (ch.arm_pose_facing * ch.facing > 0.0);
    const double walk_radius = std::max(0.05 * L, 2.0 * L - hand_reach_reduction_L * L);
    const double swing_t = std::clamp(ch.arm_phase, 0.0, 1.0);
    const double speed_ref = std::max(1.0e-6, physics_config.walk_max_speed);
    const double speed_norm = std::clamp(std::abs(cm.velocity.x) / speed_ref, 0.0, 1.0);
    const double arc_amp_scale = 1.0 - hand_speed_arc_gain * (1.0 - speed_norm);
    const double front_angle = scaleArcAngle(front_hand_start_deg,
                                             front_hand_end_deg,
                                             swing_t, arc_amp_scale);
    const double back_angle  = scaleArcAngle(back_hand_start_deg,
                                             back_hand_end_deg,
                                             swing_t, arc_amp_scale);
    const Vec2 hand_front_walk_target = armCirclePoint(shoulder_root, body_right, body_up, walk_radius, front_angle);
    const Vec2 hand_back_walk_target  = armCirclePoint(shoulder_root, body_right, body_up, walk_radius, back_angle);

    auto updateOneArm = [&](Vec2 target, Vec2 shoulder, Vec2 bend_pref,
                            const std::optional<Vec2>& dragged_target,
                            const std::optional<Vec2>& previous_elbow,
                            Vec2& elbow_out, Vec2& hand_out) {
        if (dragged_target.has_value()) {
            target = *dragged_target;
        }

        ArmPose pose;
        solveTwoBoneArm(target, upper_len, fore_len, shoulder, bend_pref, previous_elbow, pose);
        elbow_out = pose.elbow;
        hand_out  = pose.hand;
    };

    const Vec2 left_auto_target  = (ch.facing > 0.0 ? hand_back_walk_target  : hand_front_walk_target);
    const Vec2 right_auto_target = (ch.facing > 0.0 ? hand_front_walk_target : hand_back_walk_target);

    if (ch.facing > 0.0) {
        updateOneArm(left_auto_target, ch.shoulder_left, elbow_bend_pref,
                     left_hand_target,
                     keep_branch_memory ? std::optional<Vec2>(ch.elbow_left) : std::nullopt,
                     ch.elbow_left, ch.hand_left);
        updateOneArm(right_auto_target, ch.shoulder_right, elbow_bend_pref,
                     right_hand_target,
                     keep_branch_memory ? std::optional<Vec2>(ch.elbow_right) : std::nullopt,
                     ch.elbow_right, ch.hand_right);
    } else {
        updateOneArm(left_auto_target, ch.shoulder_left, elbow_bend_pref,
                     left_hand_target,
                     keep_branch_memory ? std::optional<Vec2>(ch.elbow_left) : std::nullopt,
                     ch.elbow_left, ch.hand_left);
        updateOneArm(right_auto_target, ch.shoulder_right, elbow_bend_pref,
                     right_hand_target,
                     keep_branch_memory ? std::optional<Vec2>(ch.elbow_right) : std::nullopt,
                     ch.elbow_right, ch.hand_right);
    }
    ch.arm_pose_initialized = true;
    ch.arm_pose_facing = ch.facing;
}
