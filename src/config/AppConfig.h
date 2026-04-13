#pragma once

/**
 * @file AppConfig.h
 * @brief Structures de configuration persistées dans `data/config.ini`.
 */

/** @brief Paramètres de la boucle de simulation fixe. */
struct SimLoopConfig {
    int    max_fps    = 60;
    double fixed_dt_s = 1.0 / 60.0;
    double time_scale = 1.0;
};

/** @brief Paramètres de suivi et de zoom de la caméra 2D. */
struct CameraConfig {
    double zoom       = 1.0;
    bool   follow_x   = true;
    bool   follow_y   = false;
    double smooth_x   = 0.0;   // seconds time constant, 0 = instant
    double smooth_y   = 0.0;
};

/** @brief Paramètres morphologiques globaux du personnage. */
struct CharacterConfig {
    double body_height_m   = 1.80;  // H
    double cm_pelvis_ratio = 0.75;  // CM is this many L above pelvis, range [0.60, 0.85]
    bool   show_pelvis_reach_disk = true;
    // derived (not stored): L = H/5,  h_nominal = computeNominalY(L, d_pref, ratio)
};

/** @brief Paramètres de reconstruction du buste à partir de l'état du CM. */
struct CharacterReconstructionConfig {
    double facing_eps        = 0.10;  // m/s  — deadzone to freeze facing flip
    double walk_eps          = 0.10;  // m/s  — threshold Standing ↔ Walking
    double theta_max_deg     =  6.0;  // °    — max lean angle at high speed
    double v_ref             = 1.50;  // m/s  — tanh half-saturation speed for lean
    double tau_lean          = 0.15;  // s    — lean angle filter time constant
    double tau_slope         = 0.10;  // s    — terrain slope low-pass filter
    double slope_lean_factor = 0.25;  // [-]  fraction of slope that transfers to body tilt
};

/** @brief Paramètres cinématiques et debug de la tête. */
struct HeadConfig {
    double center_offset_L  = 0.50;  // [×L] torso_top -> head center distance
    double radius_L         = 0.50;  // [×L] head radius
    double eye_height_ratio = 0.72;  // [0-1] eye line from bottom of head
    double eye_spacing      = 0.35;  // [×radius] half-spacing between eyes
    double max_tilt_deg     = 25.0;  // [deg] max head tilt toward gaze
    double tau_tilt         = 0.12;  // [s] low-pass time constant
    bool   show_eye_marker  = true;
    bool   show_gaze_ray    = true;
    bool   show_gaze_target = true;
};

/** @brief Paramètres cinématiques et debug des bras. */
struct ArmConfig {
    double upper_arm_L                    = 1.00;   // [×L] torso_top to elbow length
    double fore_arm_L                     = 1.00;   // [×L] elbow to hand length
    double walk_hand_reach_reduction_L    = 0.40;   // [×L] retract hand target from the 2L circle while walking
    double walk_front_hand_start_deg      = -20.0;  // [deg] front-hand arc start angle in torso local frame
    double walk_front_hand_end_deg        = -65.0;  // [deg] front-hand arc end angle in torso local frame
    double walk_back_hand_start_deg       = -115.0; // [deg] back-hand arc start angle in torso local frame
    double walk_back_hand_end_deg         = -165.0; // [deg] back-hand arc end angle in torso local frame
    double walk_hand_phase_speed_scale    = 0.50;   // [-] fraction of step_speed used as hand cycle Hz
    double walk_hand_speed_arc_gain       = 0.35;   // [-] how much low speed shrinks hand arc amplitude
    double walk_hand_phase_response       = 10.0;   // [s^-1] how quickly swing angular velocity follows walking intent
    double walk_hand_phase_friction       = 3.5;    // [s^-1] angular damping applied after releasing walking input

    bool   show_debug_reach_circles       = false;
    bool   show_debug_swing_points        = false;
    bool   show_debug_swing_arcs          = false;
};

/** @brief Paramètres du renderer spline expérimental. */
struct SplineRenderConfig {
    bool   enabled              = false;
    bool   draw_under_legacy    = false;
    float  stroke_width_px      = 8.0f;
    int    samples_per_curve    = 24;
    bool   show_test_curve      = true;
    bool   show_control_polygon = false;
    bool   show_sample_points   = false;
};

/** @brief Paramètres d'affichage et d'overlay liés au centre de masse. */
struct CMConfig {
    bool show_ground_reference   = true;
    bool show_projection_line    = true;
    bool show_projection_dot     = true;
    bool show_target_height_tick = true;
    // 0 = off, 1 = horizontal only, 2 = vertical only, 3 = both components
    int  velocity_components  = 3;
    int  accel_components     = 3;
    // Trail
    bool   show_trail      = false;
    double trail_duration  = 3.0;   // seconds
    // Extra debug overlays
    bool show_xcom_line           = true;   // vertical magenta line at XCoM
    bool show_support_line        = true;   // horizontal segment between planted feet
};

/** @brief Fenêtre d'échantillonnage du terrain utilisée pour la référence de sol. */
struct TerrainSamplingConfig {
    double w_back    = 0.5;   // [×L]  fixed backward sample distance
    double w_fwd     = 0.5;   // [×L]  base forward sample distance
    double t_look    = 0.20;  // [s]   velocity lookahead added to forward window
    double tau_slide = 0.12;  // [s]   time constant for endpoint sliding (exp lag)
};

/** @brief Paramètres physiques du noyau de locomotion. */
struct PhysicsConfig {
    bool   gravity_enabled  = true;
    double gravity          = 9.81;   // m/s²

    // Horizontal locomotion (stickman3-style kinematic model)
    double accel            =  6.0;   // m/s²  — horizontal acceleration while key held
    double walk_max_speed   =  1.5;   // m/s   — maximum walking speed cap
    double floor_friction   =  4.0;   // s⁻¹   — vel *= (1 - friction*dt) when not accelerating
    double hold_speed       =  0.4;   // m/s   — below this, slope gravity is suppressed at rest
    double stop_speed       =  0.05;  // m/s   — below this velocity snaps to zero (deadzone)

    // Vertical tracking — tanh-based nonlinear soft constraint
    bool   spring_enabled = true;   // toggle vertical tracking
    double vy_max         =  2.0;   // m/s   — max vertical correction speed (asymptote)
    double d_soft         =  0.15;  // m     — half-saturation distance (knee of tanh)
    double vy_tau         = 20.0;   // s⁻¹   — how fast vy adapts to vy_want

    double jump_impulse     =  5.5;   // m/s   — upward velocity on jump (Z key)
};

/** @brief Paramètres de génération du terrain procédural. */
struct TerrainConfig {
    bool   enabled     = false;
    int    seed        = 42;
    double seg_min     = 4.0;   // m
    double seg_max     = 14.0;  // m
    double angle_small = 8.0;   // °
    double angle_large = 20.0;  // °
    double large_prob  = 0.25;
    double slope_max   = 25.0;  // °
    double height_min  = -2.0;  // m
    double height_max  =  3.0;  // m
};

/** @brief Paramètres géométriques du régime debout. */
struct StandingConfig {
    double d_pref          = 0.90;  // [×L] preferred foot separation
    double d_min           = 0.75;  // [×L] minimum foot separation
    double d_max           = 1.20;  // [×L] maximum foot separation
    double eps_v           = 0.15;  // [m/s] max |vx| for standing validity
    double delta_support   = 0.20;  // [×L] support degradation band
    double k_ankle_factor  = 0.50;  // [-]  abstract ankle torque factor
};

/** @brief Paramètres du profil de levée du pied en swing. */
struct StepConfig {
    double h_clear_ratio = 0.40;  // [×L] peak height of swing arc above takeoff-landing midline
};

/** @brief Paramètres de la marche et du déclenchement des pas. */
struct WalkConfig {
    // Step trigger (window-based, stickman3-style)
    double eps_step    = 0.15;  // [m/s]  minimum speed for a step to fire
    double xcom_scale  = 0.5;   // [0-1]  scales v/ω₀ in ξ = x_cm + α·v/ω₀ (1=full capture point, 0=no lookahead)
    double d_rear_max  = 1.5;   // [×L]   max distance rear foot may lag behind pelvis before forcing a step
    double max_step_L  = 2.0;   // [×L]   max step length measured from stance foot to landing foot

    // Swing animation
    double step_speed = 5.5;  // [steps/s]  swing_t advances at this rate per second

    // Foot target — dynamic per-frame planning
    double stability_margin = 1.5;  // [×L]  foot lands this far ahead of xi (ξ)


    // CoM vertical bob — inverted pendulum arc model
    // At mid-stance (CM over foot): y_cm = y_foot + R_bob
    // R_bob = (2 − leg_flex_coeff + cm_pelvis_ratio) · L
    //   → pelvis-to-foot at mid-stance = (2 − leg_flex_coeff) · L
    // bob_scale amplifies the descent away from that peak.
    // bob_amp caps the maximum drop (hard limit on how far the CM dips).
    double leg_flex_coeff = 0.05;  // [×L] knee bend at mid-stance (0=fully ext, 0.1=10% bent)
    double bob_scale      = 3.0;   // [×]  IP arc deviation multiplier (0=flat, >1=expressive)
    double bob_amp        = 0.15;  // [×L] max drop cap below R_bob

    // Swing foot lift — computed dynamically at step initiation
    // h_clear = h_clear_ratio*L  +  h_clear_slope*L*step_slope  +  h_clear_speed*L*(|vx|/vmax)
    // h_clear_ratio is in StepConfig (base lift, already exists).
    // On downhill, step_slope < 0 → h_clear decreases. Floor: h_clear_min * L.
    double h_clear_slope_factor = 0.50;  // [×L per unit slope] extra lift going uphill
    double h_clear_speed_factor = 0.10;  // [×L at vmax]        extra lift at full speed
    double h_clear_min_ratio    = 0.05;  // [×L]                minimum foot lift (anti-drag)

    // Double support: minimum time both feet stay grounded after heel-strike
    double double_support_time = 0.06;  // [s] cooldown after heel-strike before next step fires

    // CoM height adjustments
    double cm_height_offset = 0.0;   // [m]   direct height offset (+ = taller, - = shorter)
    double max_speed_drop   = 0.15;  // [×L]  max CM drop at walk_max_speed (speed-dependent crouch)
    double max_slope_drop   = 0.20;  // [×L]  max CM drop at uphill_angle_deg slope
    double downhill_crouch_max = 0.35;  // [×L] extra CoM drop when descending
    double downhill_crouch_tau = 0.25;  // [s]  crouch engage time constant
    double downhill_relax_tau  = 0.45;  // [s]  crouch release time constant
    double downhill_step_bonus = 0.35;  // [×L] extra step length / margin while crouched
};

/** @brief Agrégat racine de toute la configuration de l'application. */
struct AppConfig {
    SimLoopConfig                 sim_loop;
    CameraConfig                  camera;
    CharacterConfig               character;
    CharacterReconstructionConfig reconstruction;
    HeadConfig                    head;
    ArmConfig                     arms;
    SplineRenderConfig            spline_render;
    CMConfig                      cm;
    PhysicsConfig                 physics;
    TerrainConfig                 terrain;
    TerrainSamplingConfig         terrain_sampling;
    StandingConfig                standing;
    StepConfig                    step;
    WalkConfig                    walk;
};
