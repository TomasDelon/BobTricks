#pragma once

/**
 * @file CharacterState.h
 * @brief État dérivé complet du personnage reconstruit autour du centre de masse.
 */

#include "core/math/Vec2.h"
#include "core/character/CMState.h"
#include "core/character/FootState.h"
#include "config/AppConfig.h"

/**
 * @brief Régime locomoteur courant du personnage.
 *
 * Utilisé dans `CharacterState` et `TelemetryRow` pour classifier le mouvement.
 */
enum class LocomotionState {
    Standing, ///< Personnage immobile, les deux pieds au sol.
    Walking,  ///< Marche à vitesse modérée.
    Running,  ///< Course (SLIP approximé, phases de vol).
    Airborne  ///< En l'air (saut ou chute libre).
};

/**
 * @brief État dérivé complet du personnage reconstruit autour du centre de masse.
 *
 * Cette structure rassemble la pose visualisable (squelette), les états des
 * membres, les filtres persistants (lean, slope, crouch) et les champs de
 * debug/télémétrie. Elle est mise à jour à chaque pas de simulation par
 * `SimulationCore::step()` et ne contient aucune donnée autoritaire sur la
 * physique (qui reste dans `CMState`).
 */
struct CharacterState {
    LocomotionState locomotion_state = LocomotionState::Standing;

    double facing = 1.0;   // +1 = droite, -1 = gauche
    double theta  = 0.0;   // angle d'inclinaison filtre (rad), utilise pour reconstruire la colonne
    double filtered_slope = 0.0;  // pente du terrain filtree, utilisee par la cible d'inclinaison
    double airborne_lean_blend = 0.0; // [0,1] melange lean avant au sol et lean arriere en l'air

    // Pose derivee : reconstruite a chaque frame depuis le CM
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
    double head_radius = 0.0;
    double arm_phase   = 0.0;
    double arm_phase_velocity = 0.0;
    double arm_run_blend = 0.0;
    bool   arm_pose_initialized = false;
    double arm_pose_facing      = 1.0;

    // Pieds et jambes
    bool      feet_initialized = false;
    Vec2      knee_left        = {0.0, 0.0};
    Vec2      knee_right       = {0.0, 0.0};
    FootState foot_left;
    FootState foot_right;

    // Auto-stepping
    double step_cooldown = 0.0;  // [s] evite un nouveau declenchement immediat apres un pas
    bool   recovery_followthrough_active = false;
    double recovery_followthrough_dir    = 0.0;  // +1 / -1 quand un swing correctif doit accompagner le CM
    double downhill_crouch               = 0.0;  // [0,1] accroupissement filtre en descente / etat de portee
    double landing_recovery_timer        = 0.0;  // [s] courte fenetre post-contact pour recuperer plus vite
    double landing_recovery_boost        = 0.0;  // [-] boost de vitesse de pas proportionnel a l'impact
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
    bool   run_mode  = false; // vrai quand run_blend > 0.5
    double run_phase = 0.0;   // [0,1) phase continue de foulee, avance avec la vitesse du CM
    bool   run_last_touchdown_left = false;

    // Echantillons persistants de reference terrain. Ils sont stockes dans
    // l'ordre gauche/droite monde pour glisser continument meme si facing change.
    bool   ground_reference_initialized = false;
    double ground_left_x                = 0.0;
    double ground_right_x               = 0.0;

    // Debug / telemetry
    bool   debug_on_floor    = false;
    double debug_cm_target_y = 0.0;
    double debug_ref_ground  = 0.0;  // reference de sol lissee (moyenne autour de +/-L)
    double debug_ref_slope   = 0.0;
    double debug_h_ip        = 0.0;
    double debug_speed_drop  = 0.0;
    double debug_slope_drop  = 0.0;
    double debug_cm_offset   = 0.0;
    // Extremites d'echantillonnage de la reference de sol apres clamp de chaque
    // intervalle contre le disque de portee centre sur le bassin.
    Vec2   debug_ground_back = {0.0, 0.0};
    Vec2   debug_ground_fwd  = {0.0, 0.0};
};

/**
 * @brief Reconstruit la pose du tronc (bassin, torse, tête) à partir du CM.
 *
 * Met à jour le régime locomoteur, l'angle de lean filtré, la direction du
 * personnage et toutes les positions de squelette dérivées. Cette fonction
 * est appelée une fois par pas de simulation dans `SimulationCore`.
 *
 * @param character    État du personnage (modifié en place).
 * @param cm           État cinématique autoritatif du CM.
 * @param config       Configuration morphologique (taille, ratios).
 * @param reconstruction Paramètres de reconstruction du torse (lean, hunch…).
 * @param on_floor     Vrai si le CM est considéré au contact du sol.
 * @param run_mode     Vrai si le mode course est actif.
 * @param dt           Pas de temps de simulation (s) — pour les filtres.
 * @param terrain_slope Pente locale du terrain (dy/dx) sous le personnage.
 */
void updateCharacterState(CharacterState&       character,
                          const CMState&        cm,
                          const CharacterConfig& config,
                          const CharacterReconstructionConfig& reconstruction,
                          bool   on_floor,
                          bool   run_mode,
                          double dt,
                          double terrain_slope);
