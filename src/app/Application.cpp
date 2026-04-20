#include "app/Application.h"

#include <cmath>
#include <optional>
#include <SDL2/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "config/ConfigIO.h"
#include "core/simulation/InputFrame.h"
#include "core/simulation/WorldConstants.h"

static constexpr int         WINDOW_W    = 1280;
static constexpr int         WINDOW_H    = 720;
static constexpr const char* CONFIG_PATH = "data/config.ini";
static constexpr const char* FOOTSTEP_WAV_PATH = "data/audio/footstep.wav";
static constexpr double      GROUND_Y    = World::GROUND_Y;
static constexpr double ACCEL_DISPLAY_SCALE = 1.0 / 9.81;

bool Application::init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    m_window = SDL_CreateWindow(
        "BobTricks V4",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!m_window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer)
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
    if (!m_renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return false;
    }

    SDL_SetWindowMinimumSize(m_window, 640, 360);

    ConfigIO::load(CONFIG_PATH, m_config);
    m_core = std::make_unique<SimulationCore>(m_config);

    SimulationLoop::Config slCfg;
    slCfg.fixed_dt_s = m_config.sim_loop.fixed_dt_s;
    m_simLoop = SimulationLoop(slCfg);
    m_simLoop.setTimeScale(m_config.sim_loop.time_scale);

    Camera2D::Config camCfg;
    m_camera = Camera2D(camCfg);
    m_camera.reset({0.0, 0.0});
    m_camera.zoomBy(m_config.camera.zoom);

    if (!m_audioSystem.init() || !m_audioSystem.loadFootstepSample(FOOTSTEP_WAV_PATH))
        SDL_Log("Audio init disabled: %s", SDL_GetError());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // This app does not persist ImGui window layout between runs.
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_renderer);
    ImGui_ImplSDLRenderer2_Init(m_renderer);

    m_prev_counter = SDL_GetPerformanceCounter();
    m_running = true;
    return true;
}

int Application::run()
{
    while (m_running) {
        const std::uint64_t frame_start = SDL_GetPerformanceCounter();

        SDL_Event event;
        while (SDL_PollEvent(&event))
            handleEvent(event);

        const std::uint64_t now  = SDL_GetPerformanceCounter();
        const double        freq = static_cast<double>(SDL_GetPerformanceFrequency());
        m_frame_dt_s   = static_cast<float>((now - m_prev_counter) / freq);
        m_prev_counter = now;
        m_current_fps  = (m_frame_dt_s > 0.f) ? (1.f / m_frame_dt_s) : 0.f;

        m_simLoop.runFrame(static_cast<double>(m_frame_dt_s),
                           [this](double dt) { stepSimulation(dt); });

        // Compute the camera target_y so that the CM appears at vertical screen center.
        // From worldToScreen: screen_y = vh - gm - ((wy - gy) - cy) * ppm
        // Solving for cy with screen_y = vh/2:
        //   cy = (cm_y - ground_y) - (vh/2 - ground_margin) / ppm
        {
            int cvw, cvh;
            SDL_GetRendererOutputSize(m_renderer, &cvw, &cvh);
            const double ppm = m_camera.getPixelsPerMeter();
            const double gm  = m_camera.getGroundMarginPx();
            const Vec2&  cm_pos       = m_core->state().cm.position;
            const double cam_target_y = (cm_pos.y - GROUND_Y)
                                      - (cvh * 0.5 - gm) / ppm;
            m_camera.update(
                static_cast<double>(m_frame_dt_s),
                cm_pos.x, cam_target_y,
                m_config.camera.follow_x, m_config.camera.follow_y,
                m_config.camera.smooth_x, m_config.camera.smooth_y,
                m_config.camera.deadzone_x, m_config.camera.deadzone_y
            );
        }

        render();
        applyFrameRateLimit(frame_start);
    }

    shutdown();
    return 0;
}

void Application::stepBack()
{
    if (m_history.empty()) return;
    const StepSnapshot& snap = m_history.back();
    m_core->loadState(snap.state);
    m_simLoop.setTotalStepCount(snap.step_count);
    m_history.pop_back();
    m_trail.clear();
    m_effectsSystem.clear();
    m_simLoop.setPaused(true);
}

void Application::stepSimulation(double dt)
{
    // Snapshot state before this step (enables step-back).
    if (m_history.size() >= MAX_HISTORY) m_history.pop_front();
    m_history.push_back({ m_core->state(), m_simLoop.getTotalStepCount() });

    InputFrame input = m_inputController.consumeInputFrame();
    m_core->step(dt, input);

    // Trail — record CM position, prune old entries.
    const SimState& s = m_core->state();
    const double sim_time = m_simLoop.getSimulationTime();
    if (m_config.cm.show_trail) {
        m_trail.push_back({sim_time, s.cm.position});
        while (!m_trail.empty()
               && sim_time - m_trail.front().time > m_config.cm.trail_duration)
            m_trail.pop_front();
    } else {
        m_trail.clear();
    }

    if (s.character.feet_initialized) {
        if (s.events.left_touchdown)
            m_audioSystem.playTouchdown(true);
        if (s.events.right_touchdown)
            m_audioSystem.playTouchdown(false);
        if (s.events.landed_from_jump)
            m_audioSystem.playLanding(std::min(1.6f, 1.0f + 0.20f * std::abs(static_cast<float>(s.cm.velocity.y))));
        if (s.events.left_slide_active)
            m_audioSystem.playSlide(true);
        if (s.events.right_slide_active)
            m_audioSystem.playSlide(false);
    }
    m_effectsSystem.update(s, m_config.particles, sim_time);
}

void Application::handleEvent(const SDL_Event& event)
{
    if (!m_inputController.isGameView())
        ImGui_ImplSDL2_ProcessEvent(&event);
    const ImGuiIO& io = ImGui::GetIO();
    const bool ui_captures_mouse = !m_inputController.isGameView() && io.WantCaptureMouse;
    const InputController::EventResult result =
        m_inputController.handleEvent(event, m_camera, m_config.camera,
                                      *m_core, m_renderer, GROUND_Y, ui_captures_mouse);
    if (result.quit_requested)
        m_running = false;
}

void Application::render()
{
    int vw = 0, vh = 0;
    SDL_GetRendererOutputSize(m_renderer, &vw, &vh);

    const SimState& s = m_core->state();
    const Terrain& terrain = m_core->terrain();
    if (!m_inputController.isGameView()) {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        const FrameStats stats{ m_current_fps, m_frame_dt_s };
        const AppRequests req = m_debugUI.render(
            stats, m_simLoop, m_config.sim_loop,
            m_camera, m_config.camera,
            m_config.character,
            m_config.head,
            m_config.arms,
            m_config.spline_render,
            m_config.presentation,
            m_config.reconstruction,
            s.cm, m_config.cm,
            s.character,
            m_config.standing,
            m_config.physics,
            m_config.terrain,
            m_config.particles,
            m_config.terrain_sampling,
            m_config.walk,
            m_config.jump,
            terrain
        );

        if (req.step_back)          stepBack();
        if (req.regenerate_terrain) m_core->regenerateTerrain();
        if (req.clear_trail)        m_trail.clear();

        if (req.ip_test_launch) {
            m_core->teleportCM(req.ip_test_cm_x, req.ip_test_cm_vx);
            SDL_Log("[IPTest] CM teleported to x=%.4f  vx=%.4f",
                    req.ip_test_cm_x, req.ip_test_cm_vx);
        }

        if (req.sim_loop || req.camera || req.character || req.head || req.arms || req.spline || req.presentation || req.reconstruction
            || req.walk || req.jump
            || req.cm || req.physics || req.terrain || req.particles) {
            m_config.sim_loop.time_scale = m_simLoop.getTimeScale();
            m_config.camera.zoom         = m_camera.getZoom();
            if (ConfigIO::save(CONFIG_PATH, m_config))
                SDL_Log("Config saved → %s", CONFIG_PATH);
            else
                SDL_Log("Failed to save config → %s", CONFIG_PATH);
        }

        ImGui::Render();
    }

    const double ref_h = terrain.height_at(s.cm.position.x);
    CharacterConfig render_char_config = m_config.character;
    HeadConfig render_head_config = m_config.head;
    ArmConfig render_arm_config = m_config.arms;
    CMConfig render_cm_config = m_config.cm;
    SplineRenderConfig render_spline_config = m_config.spline_render;

    if (m_inputController.isGameView())
        applyPresentationModeOverrides(render_char_config, render_head_config, render_arm_config,
                                       render_cm_config, render_spline_config);

    SDL_SetRenderDrawColor(m_renderer, 18, 18, 18, 255);
    SDL_RenderClear(m_renderer);

    m_sceneRenderer.render(m_renderer, m_camera, terrain, m_effectsSystem.dustParticles(),
                           m_simLoop.getSimulationTime(), GROUND_Y, vw, vh);

    if (!m_inputController.isGameView()) {
        m_debugOverlay.renderBackground(m_renderer, m_camera, render_cm_config,
                                        m_trail, m_simLoop.getSimulationTime(),
                                        GROUND_Y, vw, vh);
    }

    m_characterRenderer.render(m_renderer, m_camera, s.cm, s.character,
                               render_char_config, render_spline_config,
                               m_config.reconstruction, render_cm_config,
                               m_inputController.isGameView(),
                               terrain, GROUND_Y, vw, vh);

    if (!m_inputController.isGameView()) {
        m_debugOverlay.renderForeground(m_renderer, m_camera, s.cm,
                                        s.character,
                                        render_char_config, render_head_config, render_arm_config,
                                        m_config.standing, render_cm_config,
                                        terrain, m_inputController.gazeTargetWorld(), ref_h, ACCEL_DISPLAY_SCALE,
                                        m_inputController.isVelocityDragActive(),
                                        m_inputController.dragMouseX(),
                                        m_inputController.dragMouseY(),
                                        GROUND_Y, vw, vh);

        if (render_cm_config.show_xcom_line && s.character.feet_initialized) {
            const bool show_target = std::abs(s.cm.velocity.x) > 0.05;
            m_debugOverlay.renderXCoM(m_renderer, m_camera,
                                      s.xi, s.xi_target_x, s.xi_trigger, show_target,
                                      terrain, GROUND_Y, vw, vh);
        }
    }

    if (!m_inputController.isGameView())
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    SDL_RenderPresent(m_renderer);
}

void Application::applyPresentationModeOverrides(CharacterConfig& charConfig,
                                                 HeadConfig& headConfig,
                                                 ArmConfig& armConfig,
                                                 CMConfig& cmConfig,
                                                 SplineRenderConfig& splineConfig) const
{
    charConfig.show_pelvis_reach_disk = false;

    if (m_config.presentation.hide_head_debug) {
        headConfig.show_eye_marker = false;
        headConfig.show_gaze_ray = false;
        headConfig.show_gaze_target = false;
    }

    if (m_config.presentation.hide_arm_debug) {
        armConfig.show_debug_reach_circles = false;
        armConfig.show_debug_swing_points = false;
        armConfig.show_debug_swing_arcs = false;
    }

    if (m_config.presentation.hide_cm_debug) {
        cmConfig.show_ground_reference = false;
        cmConfig.show_projection_line = false;
        cmConfig.show_projection_dot = false;
        cmConfig.show_target_height_tick = false;
        cmConfig.show_trail = false;
    }

    if (m_config.presentation.hide_balance_debug) {
        cmConfig.show_xcom_line = false;
        cmConfig.show_support_line = false;
    }

    if (m_config.presentation.force_spline_renderer)
        splineConfig.enabled = true;

    if (m_config.presentation.hide_spline_debug) {
        splineConfig.draw_under_legacy = false;
        splineConfig.show_test_curve = false;
        splineConfig.show_control_polygon = false;
        splineConfig.show_sample_points = false;
    }
}

void Application::applyFrameRateLimit(std::uint64_t frame_start)
{
    if (m_config.sim_loop.max_fps <= 0) return;
    const double freq        = static_cast<double>(SDL_GetPerformanceFrequency());
    const double target_s    = 1.0 / static_cast<double>(m_config.sim_loop.max_fps);
    const double elapsed_s   = static_cast<double>(SDL_GetPerformanceCounter() - frame_start) / freq;
    const double remaining_s = target_s - elapsed_s;
    if (remaining_s > 0.001)
        SDL_Delay(static_cast<std::uint32_t>(remaining_s * 1000.0));
}

void Application::shutdown()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    m_audioSystem.shutdown();
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window   = nullptr; }
    SDL_Quit();
}
