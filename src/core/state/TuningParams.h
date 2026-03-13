#pragma once

/// @brief Paramètres de locomotion ajustables pour Stand/Walk/Run.
struct LocomotionTuningParams {
    // Walk
    double walk_cycle_duration_s  = 1.00;
    double walk_step_length_ratio = 0.30; ///< ratio par rapport à leg_length
    double walk_swing_lift_ratio  = 0.08;
    double walk_pelvis_bob_ratio  = 0.03;
    double walk_torso_lean_rad    = 0.07; ///< ~4 degrés
    double walk_arm_swing_rad     = 0.31; ///< ~18 degrés

    // Run
    double run_cycle_duration_s      = 0.70;
    double run_step_length_ratio     = 0.45;
    double run_swing_lift_ratio      = 0.12;
    double run_stance_compress_ratio = 0.05;
    double run_flight_lift_ratio     = 0.04;
    double run_torso_lean_rad        = 0.17; ///< ~10 degrés
    double run_arm_swing_rad         = 0.49; ///< ~28 degrés
};

/// @brief Paramètres de géométrie du corps.
/// Toutes les valeurs sont dérivées de limb_rest_length selon le modèle stickman
/// (cf. doc/design/01_Stickman_Model.md) :
///   total_height     = 5 * L
///   leg_length       = 2 * L   (cheville → torso_bottom)
///   torso_length     = 2 * L   (torso_bottom → torso_top)
///   arm_length       = 2 * L   (torso_top → wrist)
///   head_radius      = 0.5 * L
///   cm_height_stand  = 2.85 * L  (57 % de total_height, référence littérature)
struct BodyTuningParams {
    double limb_rest_length = 0.36;

    double total_height()    const { return 5.0   * limb_rest_length; }
    double leg_length()      const { return 2.0   * limb_rest_length; }
    double torso_length()    const { return 2.0   * limb_rest_length; }
    double arm_length()      const { return 2.0   * limb_rest_length; }
    double head_radius()     const { return 0.5   * limb_rest_length; }
    double cm_height_stand() const { return 2.85  * limb_rest_length; }
};

/// @brief Ensemble complet des paramètres ajustables.
struct TuningParams {
    LocomotionTuningParams locomotion;
    BodyTuningParams       body;
};
