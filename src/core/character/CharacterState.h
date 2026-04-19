#pragma once

#include "core/math/Vec2.h"
#include "core/character/CMState.h"
#include "core/character/FootState.h"
#include "config/AppConfig.h"

/** @brief Régime locomoteur courant du personnage. */
enum class LocomotionState { Standing, Walking, Running, Airborne };

/**
 * @brief État dérivé complet du personnage reconstruit autour du centre de masse.
 *
 * Cette structure rassemble la pose visualisable, les états des membres, les
 * filtres persistants et plusieurs champs de debug/telemetry.
 */
struct CharacterState {
    LocomotionState locomotion_state = LocomotionState::Standing;

    double facing = 1.0;   // +1 = right, -1 = left
    double theta  = 0.0;   // filtered lean angle (rad) — used for spine reconstruction
    double filtered_slope = 0.0;  // low-pass terrain slope used by lean target
    double airborne_lean_blend = 0.0; // [0,1] blends velocity lean from forward-on-ground to backward-in-air

    // Derived pose — reconstructed each frame from CM
    Vec2 pelvis       = {0.0, 0.0};
    Vec2 torso_center = {0.0, 0.0};
    Vec2 torso_top    = {0.0, 0.0};
    Vec2 head_center  = {0.0, 0.0};
    Vec2 shoulder_left  = {0.0, 0.0};
    Vec2 shoulder_right = {0.0, 0.0};
    Vec2 elbow_left     = {0.0, 0.0};
    Vec2 elbow_right    = {0.0, 0.0};
    Vec2 hand_left      = {0.0, 0.0};
    Vec2 hand_right     = {0.0, 0.0};
    bool hand_left_pinned  = false;
    bool hand_right_pinned = false;
    Vec2 hand_left_target  = {0.0, 0.0};
    Vec2 hand_right_target = {0.0, 0.0};
    Vec2 eye_left     = {0.0, 0.0};
    Vec2 eye_right    = {0.0, 0.0};
    double head_radius = 0.0;
    double head_tilt   = 0.0;
    double arm_phase   = 0.0;
    double arm_phase_velocity = 0.0;
    double arm_run_blend = 0.0;
    bool   arm_pose_initialized = false;
    double arm_pose_facing      = 1.0;

    // Feet and legs
    bool      feet_initialized = false;
    Vec2      knee_left        = {0.0, 0.0};
    Vec2      knee_right       = {0.0, 0.0};
    FootState foot_left;
    FootState foot_right;

    // Auto-stepping
    double step_cooldown = 0.0;  // [s] prevents immediate re-trigger after a step
    bool   recovery_followthrough_active = false;
    double recovery_followthrough_dir    = 0.0;  // +1 / -1 while a corrective swing should carry CM along
    double downhill_crouch               = 0.0;  // [0,1] filtered downhill crouch / reach state
    double landing_recovery_timer        = 0.0;  // [s] short post-touchdown window for faster recovery steps
    double landing_recovery_boost        = 0.0;  // [-] impact-scaled step-speed boost during landing recovery
    double left_slide_emit_timer         = 0.0;  // [s] cadence timer for repeated slide events
    double right_slide_emit_timer        = 0.0;  // [s] cadence timer for repeated slide events

    // Jump protocol
    bool   jump_preload_active     = false;
    bool   jump_flight_active      = false;
    double jump_preload_t          = 0.0;
    double jump_preload_duration   = 0.0;
    double jump_preload_depth      = 0.0;
    double jump_total_flight_time  = 0.0;
    double jump_time_remaining     = 0.0;
    double jump_tuck_height        = 0.0;
    Vec2   jump_takeoff_cm_pos     = {0.0, 0.0};
    Vec2   jump_takeoff_cm_vel     = {0.0, 0.0};
    Vec2   jump_left_start         = {0.0, 0.0};
    Vec2   jump_right_start        = {0.0, 0.0};
    Vec2   jump_left_target        = {0.0, 0.0};
    Vec2   jump_right_target       = {0.0, 0.0};
    bool   jump_targets_valid      = false;
    LocomotionState jump_origin_mode = LocomotionState::Standing;

    // Run mode
    double run_blend = 0.0;   // [0,1] 0=walk, 1=run — blends parameters smoothly
    bool   run_mode  = false; // true when run_blend > 0.5
    double run_phase = 0.0;   // [0,1) continuous stride phase, advances with CM speed
    bool   run_last_touchdown_left = false;

    // Persistent terrain-reference samples. These are stored in left/right world
    // order so the reference can slide continuously even when facing changes.
    bool   ground_reference_initialized = false;
    double ground_left_x                = 0.0;
    double ground_right_x               = 0.0;

    // Debug / telemetry
    bool   debug_on_floor    = false;
    double debug_cm_target_y = 0.0;
    double debug_ref_ground  = 0.0;  // smoothed ground reference (±L average)
    double debug_ref_slope   = 0.0;
    double debug_h_ip        = 0.0;
    double debug_speed_drop  = 0.0;
    double debug_slope_drop  = 0.0;
    double debug_cm_offset   = 0.0;
    // Ground-reference terrain sample endpoints after clamping each sample range
    // against the pelvis-centered reach disk.
    Vec2   debug_ground_back = {0.0, 0.0};
    Vec2   debug_ground_fwd  = {0.0, 0.0};
};

/**
 * @brief Reconstruit la pose du tronc à partir de l'état du centre de masse.
 */
void updateCharacterState(CharacterState&       character,
                          const CMState&        cm,
                          const CharacterConfig& config,
                          const CharacterReconstructionConfig& reconstruction,
                          bool   on_floor,
                          bool   run_mode,
                          double dt,
                          double terrain_slope);
