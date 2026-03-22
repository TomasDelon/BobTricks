#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>

#include "config/AppConfig.h"
#include "core/math/Vec2.h"
#include "core/character/TrailPoint.h"
#include "core/runtime/SimulationLoop.h"
#include "core/runtime/Camera2D.h"
#include "core/simulation/SimulationCore.h"
#include "render/SceneRenderer.h"
#include "render/CharacterRenderer.h"
#include "render/DebugOverlayRenderer.h"
#include "debug/DebugUI.h"

class Application
{
public:
    bool init();
    int  run();
    void shutdown();

private:
    void handleEvent(const SDL_Event& event);
    void render();
    void applyFrameRateLimit(std::uint64_t frame_start);
    void stepBack();
    void stepSimulation(double dt);

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
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

    // AZERTY locomotion input
    bool m_key_left       = false;  // Q
    bool m_key_right      = false;  // D
    bool m_jump_requested = false;  // SPACE

    // CM trajectory trail
    std::deque<TrailPoint> m_trail;

    // Step-back history — snapshot before each fixed step
    struct StepSnapshot {
        SimState      state;
        std::uint64_t step_count;
    };
    static constexpr std::size_t MAX_HISTORY = 600;
    std::deque<StepSnapshot> m_history;
};
