#pragma once

/**
 * @file AppConfig.h
 * @brief Structures de configuration de l'application, persistées dans `data/config.ini`.
 *
 * Chaque sous-structure correspond à une section INI distincte lue et écrite par
 * `ConfigIO`. La structure racine `AppConfig` agrège toutes les sous-configs en
 * un seul objet passé par référence au noyau de simulation et aux renderers.
 *
 * Toutes les unités sont exprimées en système SI sauf indication contraire.
 * Les paramètres exprimés en `[×L]` sont des multiples de la longueur de segment
 * `L = taille / 5`.
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
    double smooth_x   = 0.0;   // constante de temps (s), 0 = instantané
    double smooth_y   = 0.0;
    double deadzone_x = 0.35;  // m  — demi-fenêtre horizontale sans recentrage
    double deadzone_y = 0.15;  // m  — demi-fenêtre verticale sans recentrage
};

/** @brief Paramètres morphologiques globaux du personnage. */
struct CharacterConfig {
    double body_height_m   = 1.80;  // H
    double cm_pelvis_ratio = 0.75;  // hauteur relative CM→bassin en multiples de L, plage [0.60, 0.85]
    bool   show_pelvis_reach_disk = true;
    // Dérivés non sérialisés : L = H/5, h_nominal = computeNominalY(L, d_pref, ratio)
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
    double hunch_min_deg     = 0.0;   // °    — lower clamp for forward hunch amount
    double hunch_max_deg     = 8.0;   // °    — upper clamp for forward hunch amount
    double hunch_current_deg = 3.0;   // °    — current static hunch amount, clamped to [min,max]
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
    double run_hand_reach_reduction_L     = 0.59;   // [×L] retract hand target from the 2L circle while running
    double run_front_hand_start_deg       = -93.0;  // [deg] front-hand arc start angle in torso local frame while running
    double run_front_hand_end_deg         = -30.7;  // [deg] front-hand arc end angle in torso local frame while running
    double run_back_hand_start_deg        = -34.9;  // [deg] back-hand arc start angle in torso local frame while running
    double run_back_hand_end_deg          = -109.8; // [deg] back-hand arc end angle in torso local frame while running
    double run_hand_phase_speed_scale     = 0.50;   // [-] fraction of step_speed used as hand cycle Hz while running
    double run_hand_speed_arc_gain        = 0.22;   // [-] how much low speed shrinks hand arc amplitude while running
    double run_hand_phase_response        = 10.0;   // [s^-1] how quickly swing angular velocity follows run intent
    double run_hand_phase_friction        = 3.5;    // [s^-1] angular damping applied after releasing run input
    double run_blend_tau                  = 0.18;   // [s] low-pass on walk->run arm blending

    bool   show_debug_reach_circles       = false;
    bool   show_debug_swing_points        = false;
    bool   show_debug_swing_arcs          = false;
};

/** @brief Paramètres du renderer spline expérimental. */
struct SplineRenderConfig {
    bool   enabled              = false;
    bool   draw_under_legacy    = false;
    bool   show_head            = true;
    bool   show_torso           = true;
    bool   show_arms            = true;
    bool   show_legs            = true;
    float  stroke_width_px      = 8.0f;
    int    samples_per_curve    = 24;
    bool   show_test_curve      = true;
    bool   show_control_polygon = false;
    bool   show_sample_points   = false;
};

/** @brief Overrides appliqués quand le mode présentation est actif. */
struct PresentationConfig {
    bool show_spline_renderer        = true;
    bool show_legacy_skeleton        = false;
    bool show_character_debug_markers = false;
    bool show_pelvis_reach_disk      = false;
    bool show_trail_overlay          = false;
    bool show_ground_reference       = false;
    bool show_cm_projection          = false;
    int  velocity_components         = 0;     ///< 0: off, 1: X, 2: Y, 3: XY.
    int  accel_components            = 0;     ///< 0: off, 1: X, 2: Y, 3: XY.
    float debug_thickness_scale      = 1.0f;
    bool show_xcom_overlay           = false;
    bool show_head_overlay           = false;
    bool show_arm_overlay            = false;
    bool show_spline_debug_overlay   = false;
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
    // Traînée
    bool   show_trail      = false;
    double trail_duration  = 3.0;   // s
    // Overlays de debug supplémentaires
    bool show_xcom_line           = true;   // ligne verticale magenta au centre de masse extrapolé
    bool show_support_line        = true;   // segment horizontal entre les pieds plantés
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

    // Locomotion horizontale (modèle cinématique inspiré de stickman3)
    double accel            =  6.0;   // m/s²  — accélération horizontale quand la touche est maintenue
    double walk_max_speed   =  1.5;   // m/s   — plafond de vitesse en marche
    double floor_friction   =  4.0;   // s⁻¹   — vel *= (1 - friction*dt) hors accélération
    double hold_speed       =  0.4;   // m/s   — sous ce seuil la gravité de pente est supprimée au repos
    double stop_speed       =  0.05;  // m/s   — sous ce seuil la vitesse est rabattue à zéro

    // Suivi vertical — contrainte douce non linéaire basée sur tanh
    bool   spring_enabled = true;   // active le suivi vertical
    double vy_max         =  2.0;   // m/s   — vitesse verticale corrective maximale
    double d_soft         =  0.15;  // m     — distance de demi-saturation
    double vy_tau         = 20.0;   // s⁻¹   — vitesse d'adaptation vers vy_want

    double jump_impulse     =  5.5;   // m/s   — impulsion verticale de saut
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

/** @brief Paramètres des particules visuelles de fond. */
struct ParticlesConfig {
    bool   enabled         = false;
    bool   dust_enabled    = true;
    bool   impact_enabled  = true;
    bool   slide_enabled   = true;
    bool   landing_enabled = true;
    int    dust_burst_count = 10;
    float  dust_radius_px  = 5.0f;
    float  dust_alpha      = 90.0f;
    double dust_lifetime_s = 0.40;
    double dust_speed_mps  = 0.90;
    double slide_emit_interval_s = 0.07;
    double landing_burst_scale   = 2.4;
};

/** @brief Paramètres audio runtime pour les pas et la musique. */
struct AudioConfig {
    double footstep_volume = 2.40; // [0-4] gain global des pas / glissades / réceptions
    double music_volume    = 0.25; // [0-1] volume global de la musique
    bool   music_enabled   = true;
    int    music_track     = 1;    // index dans les pistes trouvées sous data/audio/music
};

/** @brief Paramètres géométriques du régime debout. */
struct StandingConfig {
    double d_pref          = 0.90;  // [×L] écartement préféré des pieds
    double d_min           = 0.75;  // [×L] écartement minimal des pieds
    double d_max           = 1.20;  // [×L] écartement maximal des pieds
    double eps_v           = 0.15;  // [m/s] vitesse horizontale max pour rester valide en station debout
    double delta_support   = 0.20;  // [×L] bande de dégradation du support
    double k_ankle_factor  = 0.50;  // [-] facteur abstrait de couple de cheville
};

/** @brief Paramètres du profil de levée du pied en swing. */
struct StepConfig {
    double h_clear_ratio = 0.40;  // [×L] peak height of swing arc above takeoff-landing midline
};

/** @brief Paramètres de la marche et du déclenchement des pas. */
struct WalkConfig {
    // Déclenchement du pas (fenêtre de type stickman3)
    double eps_step    = 0.15;  // [m/s]  vitesse minimale pour autoriser un pas
    double xcom_scale  = 0.5;   // [0-1]  échelle appliquée à v/ω₀ dans ξ = x_cm + α·v/ω₀
    double d_rear_max  = 1.5;   // [×L]   retard max du pied arrière avant pas forcé
    double max_step_L  = 2.0;   // [×L]   longueur maximale d'un pas

    // Animation du swing
    double step_speed = 5.5;  // [steps/s] vitesse d'avancement de swing_t

    // Cible de pied — planification dynamique à chaque frame
    double stability_margin = 1.5;  // [×L] avance d'atterrissage par rapport à ξ

    // Oscillation verticale du CM — modèle d'arc de pendule inversé
    // À la mid-stance : y_cm = y_foot + R_bob
    // R_bob = (2 − leg_flex_coeff + cm_pelvis_ratio) · L
    double leg_flex_coeff = 0.05;  // [×L] flexion du genou à la mid-stance
    double bob_scale      = 3.0;   // [×] multiplicateur d'écart à l'arc
    double bob_amp        = 0.15;  // [×L] borne max de l'abaissement

    // Levée du pied en swing — calculée au lancement du pas
    double h_clear_slope_factor = 0.50;  // [×L par unité de pente] levée supplémentaire en montée
    double h_clear_speed_factor = 0.10;  // [×L à vitesse max] levée supplémentaire à pleine vitesse
    double h_clear_min_ratio    = 0.05;  // [×L] levée minimale du pied

    // Double appui : durée minimale où les deux pieds restent au sol après heel-strike
    double double_support_time = 0.06;  // [s] temps mort avant le prochain déclenchement de pas

    // Ajustements de hauteur du CM
    double cm_height_offset = 0.0;   // [m] décalage direct de hauteur (+ = plus haut, - = plus bas)
    double max_speed_drop   = 0.15;  // [×L] abaissement max du CM à walk_max_speed
    double max_slope_drop   = 0.20;  // [×L] abaissement max du CM à la pente de référence
    double downhill_crouch_max = 0.35;  // [×L] abaissement supplémentaire en descente
    double downhill_crouch_tau = 0.25;  // [s] constante de temps d'engagement
    double downhill_relax_tau  = 0.45;  // [s] constante de temps de relâchement
    double downhill_step_bonus = 0.35;  // [×L] bonus de longueur / marge de pas en descente
};

/** @brief Paramètres du mode course (SLIP approximé). */
struct RunConfig {
    // Vitesse et pas
    double max_speed        = 3.5;   // [m/s]   plafond de vitesse en course
    double accel_factor     = 1.8;   // [×]     multiplicateur appliqué à physics.accel
    double step_speed       = 6.5;   // [steps/s] fermeture du swing plus rapide qu'en marche
    double stability_margin = 0.55;  // [×L]    légère avance d'atterrissage
    double max_step_L       = 2.5;   // [×L]    longueur max depuis le pied d'appui
    double d_rear_max       = 0.9;   // [×L]    seuil de retard du pied arrière
    double xcom_scale       = 0.75;  // [0-1]   anticipation basée sur le centre de masse extrapolé

    // Oscillation du CM pilotée par la phase
    double stride_len       = 5.8;   // [×L]    foulée cible proche de la vitesse max
    double stride_len_min   = 5.4;   // [×L]    foulée à la vitesse minimale de course

    // Cadence interpolée avec le ratio de vitesse
    double cadence_spm_min  = 162.0; // [spm]   cadence à la vitesse minimale
    double cadence_spm_max  = 176.0; // [spm]   cadence à la vitesse maximale

    // Inclinaison du corps
    double theta_max_deg    = 18.0;  // [°]     inclinaison max à pleine vitesse

    // Hauteur du CM et rebond
    double leg_flex_coeff   = 0.35;  // [×L]    flexion du genou
    double bob_scale        = 2.2;   // [×]     excursion verticale réduite en course
    double bob_amp          = 0.08;  // [×L]    amplitude max de l'oscillation

    // Levée du pied en swing
    double h_clear_ratio    = 0.30;  // [×L]    levée de base
    double h_clear_min_ratio = 0.15; // [×L]    minimum garanti

    // Blend marche → course
    double blend_tau        = 0.12;  // [s]     constante de temps de transition
};

/** @brief Paramètres du système de saut (preload, vol, réception). */
struct JumpConfig {
    // Preload (accroupissement avant décollage) — durée et profondeur par mode
    double preload_dur_run    = 0.08;  // [s]   durée du preload en mode course
    double preload_dur_walk   = 0.11;  // [s]   durée du preload en marche
    double preload_dur_stand  = 0.14;  // [s]   durée du preload en stationnaire
    double preload_depth_run  = 0.18;  // [×L]  profondeur du preload en course
    double preload_depth_walk = 0.22;  // [×L]  profondeur du preload en marche
    double preload_depth_stand= 0.26;  // [×L]  profondeur du preload en stationnaire

    // Vol
    double tuck_height_ratio  = 0.24;  // [×L]  remontée max des pieds pendant le vol

    // Réception
    double landing_dur_jump   = 0.22;  // [s]   durée de la recovery après saut
    double landing_dur_walk   = 0.18;  // [s]   durée de la recovery après chute légère
    double landing_boost_base_jump  = 0.50;  // [-]   boost de base (saut)
    double landing_boost_scale_jump = 0.90;  // [-]   sensibilité à l'impact (saut)
    double landing_boost_base_walk  = 0.35;  // [-]   boost de base (marche)
    double landing_boost_scale_walk = 0.70;  // [-]   sensibilité à l'impact (marche)
};

/**
 * @brief Agrégat racine de toute la configuration de l'application.
 *
 * Cette structure est chargée depuis `data/config.ini` au démarrage par
 * `ConfigIO::load()` et sauvegardée automatiquement lorsque l'utilisateur
 * modifie un paramètre dans l'UI de debug. Elle est partagée par référence
 * entre `Application`, `SimulationCore` et tous les renderers.
 */
struct AppConfig {
    SimLoopConfig                 sim_loop;
    CameraConfig                  camera;
    CharacterConfig               character;
    CharacterReconstructionConfig reconstruction;
    HeadConfig                    head;
    ArmConfig                     arms;
    SplineRenderConfig            spline_render;
    PresentationConfig            presentation;
    CMConfig                      cm;
    PhysicsConfig                 physics;
    TerrainConfig                 terrain;
    ParticlesConfig               particles;
    AudioConfig                   audio;
    TerrainSamplingConfig         terrain_sampling;
    StandingConfig                standing;
    StepConfig                    step;
    WalkConfig                    walk;
    RunConfig                     run;
    JumpConfig                    jump;
};
