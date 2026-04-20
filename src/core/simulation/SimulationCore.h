#pragma once

#include "core/simulation/SimState.h"
#include "core/simulation/InputFrame.h"
#include "core/terrain/Terrain.h"
#include "config/AppConfig.h"

/** @brief Timing targets derived from run-mode speed for a single step. */
struct RunTimingTargets {
    double speed_ratio    = 0.0;
    double cadence_spm    = 0.0;
    double step_period    = 0.0;
    double contact_time   = 0.0;
    double flight_time    = 0.0;
    double step_length    = 0.0;
    double touchdown_ahead = 0.0;
};

/**
 * @brief Noyau autonome de physique et de locomotion.
 *
 * Cette classe possède l'état du centre de masse, l'état du personnage, le
 * terrain et le temps de simulation. Elle ne dépend d'aucune API SDL/ImGui.
 */
class SimulationCore
{
public:
    /** @brief Construit le noyau à partir d'une configuration vivante. */
    explicit SimulationCore(AppConfig& config);

    /** @brief Avance la simulation d'un pas fixe. */
    void step(double dt, const InputFrame& input);

    /** @brief Réinitialise la simulation à un état initial connu. */
    void reset(const ScenarioInit& init);

    /** @brief Recharge un instantané complet de simulation. */
    void loadState(const SimState& snap);

    /** @brief Régénère le terrain à partir de la configuration courante. */
    void regenerateTerrain();
    /** @brief Téléporte le centre de masse, utilisé notamment par le panel IP. */
    void teleportCM(double x, double vx);
    /** @brief Force la vitesse du centre de masse. */
    void setCMVelocity(Vec2 vel);
    /** @brief Épingle ou désépingle un pied. */
    void toggleFootPin(bool left);
    /** @brief Épingle ou désépingle une main. */
    void toggleHandPin(bool left);

    /** @brief Retourne l'état courant en lecture seule. */
    const SimState& state()   const { return m_state; }
    double          time()    const { return m_state.sim_time; }
    const Terrain&  terrain() const { return m_terrain; }

private:
    AppConfig& m_config;   // non-owning — Application's m_config
    Terrain    m_terrain;  // declared after m_config (stores ref to m_config.terrain)
    SimState   m_state;

    /** @brief Per-frame context threading intermediate values between step sub-phases. */
    struct StepCtx {
        // Phase 1: derived constants
        double L            = 0.0;
        double g            = 0.0;
        double h_nominal    = 0.0;
        double reach_radius = 0.0;

        // Phase 3: ground reference
        double ref_ground   = 0.0;
        double ref_slope    = 0.0;
        Vec2   ground_back  = {0.0, 0.0};
        Vec2   ground_fwd   = {0.0, 0.0};
        double sin_t        = 0.0;
        double cos_t        = 0.0;
        bool   airborne_ref = false;

        // Phase 4: input
        double input_dir    = 0.0;
        bool   prev_contact_left  = false;
        bool   prev_contact_right = false;
        bool   prev_jump_flight_active = false;

        // Phase 5-6: run mode + blended params
        double           rb  = 0.0;
        RunTimingTargets run_timing;
        WalkConfig       eff_walk;
        StepConfig       eff_step;
        PhysicsConfig    eff_physics;
        double           speed_abs = 0.0;
        double           max_spd   = 0.0;

        // Phase 7: physics integration
        Vec2   accel      = {0.0, 0.0};
        double y_tgt      = 0.0;
        double h_ip       = 0.0;
        double speed_drop = 0.0;
        double slope_drop = 0.0;
        bool   jump_vertical_override = false;
        double pre_vertical_velocity  = 0.0;
        double landing_recovery_gain   = 0.0;
        bool   landing_recovery_active = false;

        // Phase 8: post-integration geometry
        Vec2   pelvis         = {0.0, 0.0};
        bool   airborne_final = false;

        // Phase 9: swing advance
        bool was_swinging_L = false;
        bool was_swinging_R = false;
        bool heel_strike_L  = false;
        bool heel_strike_R  = false;

        // Phase 10: trigger state
        double omega0  = 0.0;
        double xi      = 0.0;
        double eff_lx  = 0.0;
        double eff_rx  = 0.0;

        // Phase 11: airborne/jump outcome
        bool any_swinging   = false;

        // Phase 13: constraint snapshot
        bool was_grounded_L = false;
        bool was_grounded_R = false;
    };

    void stepComputeConstants   (StepCtx& ctx);
    void stepBootstrapCM        (StepCtx& ctx, const InputFrame& input);
    void stepGroundReference    (StepCtx& ctx, double dt);
    void stepProcessInput       (StepCtx& ctx, const InputFrame& input);
    void stepBlendRunMode       (StepCtx& ctx, const InputFrame& input, double dt);
    void stepBlendParams        (StepCtx& ctx);
    void stepIntegratePhysics   (StepCtx& ctx, double dt);
    void stepReconstructGeometry(StepCtx& ctx, const InputFrame& input);
    void stepAdvanceSwing       (StepCtx& ctx, double dt);
    void stepComputeTriggerState(StepCtx& ctx);
    void stepAirborneJump       (StepCtx& ctx);
    void stepFireTriggers       (StepCtx& ctx);
    void stepApplyConstraints   (StepCtx& ctx);
    void stepWriteOutput        (StepCtx& ctx, const InputFrame& input, double dt);
    void stepRetargetLateSwings (StepCtx& ctx);
    void stepHandleJumpFlight   (StepCtx& ctx);
    void stepHandleGroundRecontact(StepCtx& ctx);
    void stepFireRunTrigger     (StepCtx& ctx);
    void stepFireWalkTrigger    (StepCtx& ctx);
    void stepUpdateContactEvents(StepCtx& ctx);
    void stepUpdateSlideEvents  (double dt);
    bool stepLaunchSwing        (bool step_left, bool corrective, StepCtx& ctx);
};
