#pragma once

#include "core/runtime/SimulationLoop.h"
#include "core/terrain/Terrain.h"
#include "render/Camera2D.h"
#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "config/AppConfig.h"

/**
 * @file DebugUI.h
 * @brief Interface Dear ImGui de contrôle et d'inspection en temps réel.
 */

/**
 * @brief Statistiques de frame injectées dans l'UI de debug.
 *
 * Calculées par `Application` à partir du compteur haute résolution SDL et
 * affichées dans le panneau de boucle de simulation.
 */
struct FrameStats {
    float current_fps = 0.f; ///< FPS courant (images par seconde).
    float frame_dt_s  = 0.f; ///< Durée de la dernière frame rendue (s).
};

/**
 * @brief Requêtes émises par l'UI vers `Application` après un rendu ImGui.
 *
 * Chaque champ booléen indique qu'une configuration a été modifiée et doit
 * être persistée ou appliquée par l'application. Les champs `ip_test_*` sont
 * réservés au panneau de test du pendule inversé.
 */
struct AppRequests {
    bool sim_loop  = false;
    bool camera    = false;
    bool character = false;
    bool head      = false;
    bool arms      = false;
    bool spline    = false;
    bool presentation = false;
    bool cm        = false;
    bool walk      = false;
    bool jump      = false;
    bool physics   = false;
    bool particles = false;
    bool audio     = false;
    bool terrain            = false;
    bool regenerate_terrain = false;
    bool clear_trail        = false;
    bool step_back          = false;
    bool reconstruction     = false;

    // Test de pendule inverse : l'application teleporte le CM a ces coordonnees au lancement.
    bool   ip_test_launch = false;
    double ip_test_cm_x   = 0.0;
    double ip_test_cm_vx  = 0.0;
};

/**
 * @brief Interface Dear ImGui de contrôle et d'inspection en temps réel.
 *
 * Cette classe regroupe tous les panneaux de debug de l'application :
 * simulation, caméra, physique, locomotion, bras, tête, torse, pieds,
 * splines, terrain, particules, balance, saut et test pendule inversé.
 * Elle est sans état entre deux appels à `render()`, à l'exception du panneau
 * de test du pendule inverse qui persiste `m_ipTest` pour afficher la
 * trajectoire analytique.
 */
class DebugUI
{
public:
    /**
     * @brief Dessine tous les panneaux ImGui et retourne les requêtes utilisateur.
     *
     * @return `AppRequests` indiquant quelles configurations ont été modifiées.
     */
    AppRequests render(const FrameStats&      stats,
                        SimulationLoop&        simLoop,   SimLoopConfig&   simConfig,
                        Camera2D&              camera,    CameraConfig&    camConfig,
                        CharacterConfig&       charConfig,
                        HeadConfig&            headConfig,
                        ArmConfig&             armConfig,
                        SplineRenderConfig&    splineConfig,
                        PresentationConfig&    presentationConfig,
                        CharacterReconstructionConfig& reconstructionConfig,
                        const CMState&         cmState,   CMConfig&        cmConfig,
                             const CharacterState&  charState,
                             const StandingConfig&  standConfig,
                             PhysicsConfig&         physConfig,
                             TerrainConfig&         terrainConfig,
                             ParticlesConfig&       particlesConfig,
                             AudioConfig&           audioConfig,
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
    void renderPresentationPanel(PresentationConfig& config, bool& saveRequested);
    void renderCMKinematicsPanel(const CMState& state, CMConfig& config, bool& saveRequested, bool& clearTrail);
    void renderLocomotionPanel  (const CMState& cmState, const CharacterState& charState,
                                 const CharacterConfig&,
                                 const CharacterReconstructionConfig& reconstructionConfig,
                                 const Terrain&,
                                 WalkConfig& walkConfig, bool& saveRequested);
    void renderBalancePanel     (const CMState& cmState, const CharacterState& charState,
                                 const CharacterConfig& charConfig, const StandingConfig& standConfig,
                                 CMConfig& cmConfig, bool& saveRequested);
    void renderJumpPanel    (JumpConfig&    config,  bool& saveRequested);
    void renderPhysicsPanel (PhysicsConfig& config,  bool& saveRequested);
    void renderTerrainPanel        (TerrainConfig& config,         bool& saveRequested, bool& regenerateRequested);
    void renderParticlesPanel      (ParticlesConfig& config,       bool& saveRequested);
    void renderAudioPanel          (AudioConfig& config,           bool& saveRequested);
    void renderHelpPanel           ();
    void renderTerrainSamplingPanel(TerrainSamplingConfig& config, CMConfig& cmConfig, bool& saveRequested);
    void renderIPTestPanel  (const CMState& cmState, const CharacterState& charState,
                             const CharacterConfig& charConfig, const StandingConfig& standConfig,
                             const PhysicsConfig& physConfig, SimulationLoop& simLoop,
                             AppRequests& req);

    // Etat persistant entre les frames pour le test du pendule inverse.
    struct IPTestState {
        bool   active    = false;
        double t_start   = 0.0;   // temps de simulation au lancement
        double stance_x  = 0.0;   // abscisse du pied d'appui au lancement
        double facing    = 1.0;
        double x0_rel    = 0.0;   // valeur initiale cm.x - stance.x (= 0.3L)
        double v0        = 0.0;   // valeur initiale de cm.vx (= 0)
        double omega0    = 0.0;   // sqrt(g / nominal_y)
        double mu        = 0.0;   // floor_friction
        double r1        = 0.0;   // racine positive de r²+mu·r-ω0²=0
        double r2        = 0.0;   // racine negative
        double C1        = 0.0;   // coefficient de solution
        double C2        = 0.0;
    } m_ipTest;
};
