#pragma once

/// @brief Paramètres de locomotion ajustables pour Stand/Walk/Run.
struct LocomotionTuningParams {
    // Walk
    double walk_cycle_duration_s  = 1.0;
    double walk_step_length_ratio = 0.30; ///< ratio par rapport à la longueur de jambe
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
struct BodyTuningParams {
    double total_height   = 1.80; ///< hauteur totale du personnage (m)
    double leg_length     = 0.50; ///< longueur d'une jambe (m)
    double torso_length   = 0.40; ///< longueur du tronc (m)
    double arm_length     = 0.35; ///< longueur d'un bras (m)
    double head_radius    = 0.12; ///< rayon de la tête (m)
};

/// @brief Ensemble complet des paramètres ajustables.
struct TuningParams {
    LocomotionTuningParams locomotion;
    BodyTuningParams       body;
};
