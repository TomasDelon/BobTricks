#pragma once

#include "core/math/Vec2.h"
#include "core/character/CMState.h"
#include "core/character/FootState.h"
#include "config/AppConfig.h"

/** @brief Régime locomoteur courant du personnage. */
enum class LocomotionState { Standing, Walking, Airborne };

/**
 * @brief État dérivé complet du personnage reconstruit autour du centre de masse.
 *
 * Cette structure rassemble la pose visualisable, les états des membres, les
 * filtres persistants et plusieurs champs de debug/telemetry.
 *
 * Les champs de pose sont recalculés à chaque pas fixe. Les champs de debug
 * persistent assez longtemps pour être lus par l'UI, l'overlay et la
 * télémétrie headless.
 */
struct CharacterState {
    LocomotionState locomotion_state = LocomotionState::Standing;

    double facing = 1.0;   // +1 = right, -1 = left
    double theta  = 0.0;   // filtered lean angle (rad) — used for spine reconstruction
    double filtered_slope = 0.0;  // low-pass terrain slope used by lean target

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
 * @param character       État du personnage à mettre à jour.
 * @param cm              État du centre de masse courant.
 * @param config          Paramètres morphologiques globaux.
 * @param reconstruction  Paramètres de reconstruction du tronc.
 * @param on_floor        Indique si le centre de masse est considéré au sol.
 * @param dt              Pas fixe courant.
 * @param terrain_slope   Pente locale du terrain sous le personnage.
 */
void updateCharacterState(CharacterState&       character,
                          const CMState&        cm,
                          const CharacterConfig& config,
                          const CharacterReconstructionConfig& reconstruction,
                          bool   on_floor,
                          double dt,
                          double terrain_slope);
