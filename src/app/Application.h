#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

#include "config/AppConfig.h"
#include "app/AudioSystem.h"
#include "app/EffectsSystem.h"
#include "app/InputController.h"
#include "core/math/Vec2.h"
#include "core/character/TrailPoint.h"
#include "core/runtime/SimulationLoop.h"
#include "render/Camera2D.h"
#include "core/simulation/SimulationCore.h"
#include "render/SceneRenderer.h"
#include "render/CharacterRenderer.h"
#include "render/DebugOverlayRenderer.h"
#include "debug/DebugUI.h"

/**
 * @file Application.h
 * @brief Orchestrateur principal de l'application SDL interactive.
 */

/**
 * @brief Application SDL interactive principale.
 *
 * Cette classe possède la fenêtre SDL2, le renderer, la boucle de simulation
 * à pas fixe, la caméra, l'UI de debug ImGui et tous les renderers visuels.
 * Elle orchestre le flux complet : événements SDL → `InputController` →
 * `SimulationCore` → renderers → ImGui → présentation.
 *
 * `SimulationCore` est un membre direct de `Application` : il prend une
 * référence sur `m_config`, puis son terrain est régénéré après chargement du
 * fichier de configuration.
 */
class Application
{
public:
    /** @brief Construit les sous-objets dépendants de la configuration. */
    Application();

    /** @brief Initialise SDL, ImGui, la configuration et le noyau de simulation. */
    bool init();
    /** @brief Lance la boucle principale jusqu'à fermeture de l'application. */
    int  run();
    /** @brief Libère proprement les ressources SDL et ImGui. */
    void shutdown();

private:
    void handleEvent(const SDL_Event& event);
    void render();
    void applyFrameRateLimit(std::uint64_t frame_start);
    void stepBack();
    void stepSimulation(double dt);
    void applyPresentationModeOverrides(CharacterConfig& charConfig,
                                        HeadConfig& headConfig,
                                        ArmConfig& armConfig,
                                        CMConfig& cmConfig,
                                        SplineRenderConfig& splineConfig) const;
    bool presentationForegroundOverlayEnabled() const;

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool          m_running  = false;

    AppConfig            m_config;
    SimulationCore       m_core;
    SimulationLoop       m_simLoop;
    Camera2D             m_camera;
    SceneRenderer        m_sceneRenderer;
    CharacterRenderer    m_characterRenderer;
    DebugOverlayRenderer m_debugOverlay;
    DebugUI              m_debugUI;
    AudioSystem          m_audioSystem;
    EffectsSystem        m_effectsSystem;

    std::uint64_t m_prev_counter  = 0;
    float         m_current_fps   = 0.f;
    float         m_frame_dt_s    = 0.f;

    bool m_is_panning     = false;

    // Right-click drag → set CM velocity (queued for next step via InputFrame)
    InputController m_inputController;

    // CM trajectory trail
    std::deque<TrailPoint> m_trail;

    // Step-back history — snapshot before each fixed step
    /** @brief Instantané stocké avant chaque pas fixe pour le step-back. */
    struct StepSnapshot {
        SimState      state;
        std::uint64_t step_count;
    };
    static constexpr std::size_t MAX_HISTORY = 600;
    std::deque<StepSnapshot> m_history;
};
