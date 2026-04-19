#pragma once

#include "core/runtime/SimulationLoop.h"
#include "core/terrain/Terrain.h"
#include "render/Camera2D.h"
#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "config/AppConfig.h"

/** @brief Statistiques de frame injectées dans l'UI de debug. */
struct FrameStats {
    float current_fps = 0.f;
    float frame_dt_s  = 0.f;
};

/** @brief Requêtes émises par l'UI vers l'application après un frame ImGui. */
struct AppRequests {
    bool sim_loop  = false;
    bool camera    = false;
    bool character = false;
    bool head      = false;
    bool arms      = false;
    bool spline    = false;
    bool cm        = false;
    bool walk      = false;
    bool jump      = false;
    bool physics   = false;
    bool particles = false;
    bool terrain            = false;
    bool regenerate_terrain = false;
    bool clear_trail        = false;
    bool step_back          = false;
    bool reconstruction     = false;

    // IP test: Application teleports the CM to these coords when launch=true.
    bool   ip_test_launch = false;
    double ip_test_cm_x   = 0.0;
    double ip_test_cm_vx  = 0.0;
};

/**
 * @brief Interface ImGui de contrôle et d'inspection du projet.
 */
class DebugUI
{
public:
    /** @brief Dessine la fenêtre de debug et retourne les requêtes utilisateur. */
    AppRequests render(const FrameStats&      stats,
                        SimulationLoop&        simLoop,   SimLoopConfig&   simConfig,
                        Camera2D&              camera,    CameraConfig&    camConfig,
                        CharacterConfig&       charConfig,
                        HeadConfig&            headConfig,
                        ArmConfig&             armConfig,
                        SplineRenderConfig&    splineConfig,
                        CharacterReconstructionConfig& reconstructionConfig,
                        const CMState&         cmState,   CMConfig&        cmConfig,
                             const CharacterState&  charState,
                             const StandingConfig&  standConfig,
                             PhysicsConfig&         physConfig,
                             TerrainConfig&         terrainConfig,
                             ParticlesConfig&       particlesConfig,
                             TerrainSamplingConfig& terrainSamplingConfig,
                             WalkConfig&            walkConfig,
                             JumpConfig&            jumpConfig,
                             const Terrain&         terrain);

private:
    void renderSimLoopPanel  (const FrameStats& stats, SimulationLoop& simLoop,
                              SimLoopConfig& config,   bool& saveRequested, bool& stepBack);
    void renderCameraPanel   (Camera2D& camera, CameraConfig& config, bool& saveRequested);
    void renderCharacterPanel(CharacterConfig& config, const StandingConfig& standConfig, bool& saveRequested);
    void renderHeadPanel(const CharacterState& charState, HeadConfig& config, bool& saveRequested);
    void renderReconstructionPanel(CharacterReconstructionConfig& config, bool& saveRequested);
    void renderTorsoPanel(const CharacterState& charState, const CharacterConfig& charConfig,
                          CharacterReconstructionConfig& reconstructionConfig,
                          const Terrain& terrain, bool& saveRequested);
    void renderLegsPanel(const CharacterState& charState, CharacterConfig& charConfig,
                         CMConfig& cmConfig, bool& saveRequested);
    void renderArmsPanel     (ArmConfig& config, bool& saveRequested);
    void renderSplinePanel   (SplineRenderConfig& config, bool& saveRequested);
    void renderCMKinematicsPanel(const CMState& state, CMConfig& config, bool& saveRequested, bool& clearTrail);
    void renderLocomotionPanel  (const CMState& cmState, const CharacterState& charState,
                                 const CharacterConfig& charConfig,
                                 const CharacterReconstructionConfig& reconstructionConfig,
                                 const Terrain& terrain,
                                 WalkConfig& walkConfig, bool& saveRequested);
    void renderBalancePanel     (const CMState& cmState, const CharacterState& charState,
                                 const CharacterConfig& charConfig, const StandingConfig& standConfig,
                                 CMConfig& cmConfig, bool& saveRequested);
    void renderJumpPanel    (JumpConfig&    config,  bool& saveRequested);
    void renderPhysicsPanel (PhysicsConfig& config,  bool& saveRequested);
    void renderTerrainPanel        (TerrainConfig& config,         bool& saveRequested, bool& regenerateRequested);
    void renderParticlesPanel      (ParticlesConfig& config,       bool& saveRequested);
    void renderHelpPanel           ();
    void renderTerrainSamplingPanel(TerrainSamplingConfig& config, CMConfig& cmConfig, bool& saveRequested);
    void renderIPTestPanel  (const CMState& cmState, const CharacterState& charState,
                             const CharacterConfig& charConfig, const StandingConfig& standConfig,
                             const PhysicsConfig& physConfig, SimulationLoop& simLoop,
                             AppRequests& req);

    // State persisted across frames for the IP test.
    struct IPTestState {
        bool   active    = false;
        double t_start   = 0.0;   // simulation time at launch
        double stance_x  = 0.0;   // stance foot x at launch
        double facing    = 1.0;
        double x0_rel    = 0.0;   // initial cm.x - stance.x (= 0.3L)
        double v0        = 0.0;   // initial cm.vx            (= 0)
        double omega0    = 0.0;   // sqrt(g / nominal_y)
        double mu        = 0.0;   // floor_friction
        double r1        = 0.0;   // positive root of r²+mu·r-ω0²=0
        double r2        = 0.0;   // negative root
        double C1        = 0.0;   // solution coefficient
        double C2        = 0.0;
    } m_ipTest;
};
