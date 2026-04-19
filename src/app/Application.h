#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <vector>

#include "config/AppConfig.h"
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
 * @brief Application SDL interactive principale.
 *
 * Cette classe possède la fenêtre, le renderer SDL, la boucle de simulation,
 * la caméra, l'UI de debug et tous les renderers visuels. Elle sert
 * d'orchestrateur entre le noyau de simulation et l'interface utilisateur.
 */
class Application
{
public:
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
    bool initAudio();
    bool loadFootstepSample();
    void queueFootstep(float gain);
    void emitFootDust(const FootState& foot,
                      double sim_time,
                      double burst_scale,
                      double tangent_bias,
                      double vertical_scale);
    void emitSlideDust(const FootState& foot, double sim_time, double tangent_dir);
    void pruneDustParticles(double sim_time);

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_AudioDeviceID m_audio_device = 0;
    bool          m_running  = false;

    AppConfig                       m_config;
    std::unique_ptr<SimulationCore> m_core;    // created in init() after config load
    SimulationLoop                  m_simLoop;
    Camera2D                        m_camera;
    SceneRenderer                   m_sceneRenderer;
    CharacterRenderer               m_characterRenderer;
    DebugOverlayRenderer            m_debugOverlay;
    DebugUI                         m_debugUI;

    std::uint64_t m_prev_counter  = 0;
    float         m_current_fps   = 0.f;
    float         m_frame_dt_s    = 0.f;

    bool m_is_panning     = false;

    // Right-click drag → set CM velocity (queued for next step via InputFrame)
    bool              m_drag_vel_active    = false;
    float             m_drag_mouse_x       = 0.f;
    float             m_drag_mouse_y       = 0.f;
    std::optional<Vec2> m_pending_set_velocity;  // consumed once in stepSimulation()
    std::optional<Vec2> m_gaze_target_world;

    // Left-click drag → move foot (world-space target updated on motion)
    bool m_dragging_foot_left  = false;
    bool m_dragging_foot_right = false;
    Vec2 m_foot_drag_world     = {0.0, 0.0};
    bool m_dragging_hand_left  = false;
    bool m_dragging_hand_right = false;
    Vec2 m_hand_drag_world     = {0.0, 0.0};

    // AZERTY locomotion input
    bool m_key_left       = false;  // Q
    bool m_key_right      = false;  // D
    bool m_key_run        = false;  // Shift
    bool m_jump_requested = false;  // SPACE
    bool m_game_view      = false;  // P toggles spline-only presentation mode

    // CM trajectory trail
    std::deque<TrailPoint> m_trail;
    std::deque<DustParticle> m_dust_particles;
    std::vector<float> m_footstep_sample;

    // Step-back history — snapshot before each fixed step
    /** @brief Instantané stocké avant chaque pas fixe pour le step-back. */
    struct StepSnapshot {
        SimState      state;
        std::uint64_t step_count;
    };
    static constexpr std::size_t MAX_HISTORY = 600;
    std::deque<StepSnapshot> m_history;
};
