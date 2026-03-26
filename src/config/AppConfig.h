#pragma once

// Root config — one sub-struct per subsystem.
// Loaded from / saved to data/config.ini at startup/on demand.

struct SimLoopConfig {
    int    max_fps    = 60;
    double fixed_dt_s = 1.0 / 60.0;
    double time_scale = 1.0;
};

struct CameraConfig {
    double zoom       = 1.0;
    bool   follow_x   = true;
    bool   follow_y   = false;
    double smooth_x   = 0.0;   // seconds time constant, 0 = instant
    double smooth_y   = 0.0;
};

struct CharacterConfig {
    double body_height_m   = 1.80;  // H
    double body_mass_kg    = 60.0;
    double cm_pelvis_ratio = 0.75;  // cm is this many L above pelvis, range [0.60, 0.85]
    // derived (not stored): L = H/5,  cm_nominal_height = (2 + ratio) * L
};

struct CharacterReconstructionConfig {
    double facing_tau    = 0.12;  // s    — facing_vel filter time constant
    double facing_eps    = 0.10;  // m/s  — deadzone to freeze facing flip
    double walk_eps      = 0.10;  // m/s  — threshold Standing ↔ Walking
    double theta_max_deg = 10.0;  // °    — max pelvis lean at high speed
    double v_ref         = 1.20;  // m/s  — tanh half-saturation speed
};

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
    bool show_planted_feet_color  = true;   // feet turn orange when planted
};

struct PhysicsConfig {
    bool   gravity_enabled        = true;
    double gravity                = 9.81;  // m/s²

    bool   air_friction_enabled   = false;
    double air_friction           = 0.5;   // linear drag coefficient (s⁻¹)

    bool   floor_friction_enabled = true;
    double floor_friction         = 4.0;   // viscous floor damping (s⁻¹)

    // Leg spring — vertical spring-damper keeping CM at nominal height
    bool   spring_enabled         = true;
    double spring_stiffness       = 200.0; // s⁻²  (acc per meter of compression)
    double spring_damping         = 20.0;  // s⁻¹  (acc per m/s of velocity)

    // Locomotion input
    double move_force             =  4.0;  // m/s²  horizontal acceleration (Q/D keys)
                                          //       terminal_v = move_force / floor_friction = 1 m/s
    double jump_impulse           = 5.5;   // m/s   upward velocity on jump (Z key)
};

// Procedural terrain: random angle-walk (inspired by stickman2).
// Each step chooses a random length and angle change; soft height bounds
// keep the terrain from drifting too far.  height_at(x) returns an offset
// above GROUND_Y (0 when disabled or outside generated range).
struct TerrainConfig {
    bool   enabled     = false;
    int    seed        = 42;
    double seg_min     = 4.0;   // m — min segment length
    double seg_max     = 14.0;  // m — max segment length
    double angle_small = 8.0;   // ° — max change for a small step (high freq)
    double angle_large = 20.0;  // ° — max change for a large step (low freq)
    double large_prob  = 0.25;  // probability of choosing a large step
    double slope_max   = 25.0;  // ° — absolute angle clamp
    double height_min  = -2.0;  // m — soft lower bound
    double height_max  =  3.0;  // m — soft upper bound
};

// Standing pose geometry.
// Values marked [×L] are dimensionless ratios; multiply by L = H/5 at runtime.
// Values marked [m/s] or [s] are in SI units directly.
struct StandingConfig {
    double d_pref   = 0.90;  // [×L] preferred foot separation
    double d_min    = 0.75;  // [×L] minimum foot separation
    double d_max    = 1.20;  // [×L] maximum foot separation
    double k_leg    = 0.90;  // [-]  leg extension ratio ∈ (0,1]; r = 2L*k_leg
    double eps_v    = 0.15;  // [m/s] max |vx| for standing validity
    double k_crouch = 0.15;  // [×L] CM drop for recovery crouch
};

// Balance metric (XCoM / Hof 2008).
struct BalanceConfig {
    double h_ref_override     = -1.0;   // [m]  use auto (from geometry) if < 0
    double mos_step_threshold =  0.0;   // [×L] MoS below this triggers a step (0 = at boundary)
};

// Swing foot trajectory.
struct StepConfig {
    double h_clear_ratio    = 0.10;  // [×L] peak clearance of swing arc
    double T_min            = 0.18;  // [s]  minimum swing duration
    double T_max            = 0.30;  // [s]  maximum swing duration
    double dist_coeff       = 0.20;  // [s]  duration growth per (dist/L) unit
    // Recovery steps may use a wider separation than comfortable standing allows.
    double d_max_correction = 1.80;  // [×L] max foot separation for corrective step
    // CM vertical bobbing during swing.
    // Amplitude = k_bob * L * tanh(|vx| / v_ref_bob), applied as sin(π·φ) offset
    // to the spring target — rises at mid-swing (φ=0.5), zero at toe-off/heel-strike.
    double k_bob     = 0.10;  // [×L] max bob amplitude ratio
    double v_ref_bob = 1.00;  // [m/s] speed at which bob reaches 76% of max
};

// Walking trigger and foot placement.
// All distances in [×L] units (multiply by L = H/5 at runtime).
struct WalkConfig {
    // Trigger: step fires when rear foot is this many L behind the pelvis.
    // Must be > d_pref/2 = 0.45 to avoid firing at rest. Validated: 0.45 < 0.70. ✓
    double k_trigger = 0.70;  // [×L]

    // Target: swing foot lands k_step*L + |v_x|*T_ant ahead of the CM.
    // Placing the target relative to CM (not stance foot) avoids leg-reach infeasibility.
    double k_step    = 0.90;  // [×L] base step reach ahead of CM
    double T_ant     = 0.15;  // [s]  velocity look-ahead added to the landing target

    // Rule 3 — IK reach violation trigger.
    // Fires when the planted rear foot exceeds this absolute distance beyond
    // max IK reach (2L).  Calibrated above walking noise floor (~0.03 m) and
    // below the first visible artefact (~0.05 m).
    double reach_margin = 0.04;  // [m]
};

struct AppConfig {
    SimLoopConfig                 sim_loop;
    CameraConfig                  camera;
    CharacterConfig               character;
    CharacterReconstructionConfig reconstruction;
    CMConfig                      cm;
    PhysicsConfig                 physics;
    TerrainConfig                 terrain;
    StandingConfig                standing;
    BalanceConfig                 balance;
    StepConfig                    step;
    WalkConfig                    walk;
};
