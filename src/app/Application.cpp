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
static constexpr double      GROUND_Y    = World::GROUND_Y;
static constexpr double      ZOOM_STEP   = 1.1;
static constexpr float  CM_HIT_RADIUS_PX    = 20.f;
static constexpr float  FOOT_HIT_RADIUS_PX  = 15.f;
static constexpr float  HAND_HIT_RADIUS_PX  = 15.f;
static constexpr double ACCEL_DISPLAY_SCALE = 1.0 / 9.81;

bool Application::init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
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
                m_config.camera.smooth_x, m_config.camera.smooth_y
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
    m_simLoop.setPaused(true);
}

void Application::stepSimulation(double dt)
{
    // Snapshot state before this step (enables step-back).
    if (m_history.size() >= MAX_HISTORY) m_history.pop_front();
    m_history.push_back({ m_core->state(), m_simLoop.getTotalStepCount() });

    // Build input frame and advance the simulation core.
    InputFrame input;
    input.key_left       = m_key_left;
    input.key_right      = m_key_right;
    input.key_run        = m_key_run && (m_key_left || m_key_right);
    input.jump           = m_jump_requested;
    input.set_velocity   = m_pending_set_velocity;
    input.gaze_target_world = m_gaze_target_world;
    m_jump_requested     = false;
    m_pending_set_velocity.reset();

    if (m_dragging_foot_left) {
        input.foot_left_drag = true;
        input.foot_left_pos  = m_foot_drag_world;
    }
    if (m_dragging_foot_right) {
        input.foot_right_drag = true;
        input.foot_right_pos  = m_foot_drag_world;
    }
    if (m_dragging_hand_left) {
        input.hand_left_drag = true;
        input.hand_left_pos  = m_hand_drag_world;
    }
    if (m_dragging_hand_right) {
        input.hand_right_drag = true;
        input.hand_right_pos  = m_hand_drag_world;
    }

    m_core->step(dt, input);

    // Trail — record CM position, prune old entries.
    const SimState& s = m_core->state();
    if (m_config.cm.show_trail) {
        const double now = m_simLoop.getSimulationTime();
        m_trail.push_back({now, s.cm.position});
        while (!m_trail.empty()
               && now - m_trail.front().time > m_config.cm.trail_duration)
            m_trail.pop_front();
    } else {
        m_trail.clear();
    }
}

void Application::handleEvent(const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
    const ImGuiIO& io = ImGui::GetIO();

    if (event.type == SDL_QUIT)
        m_running = false;
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        m_running = false;

    // ZQSD locomotion (AZERTY layout)
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        const bool pressed = (event.type == SDL_KEYDOWN);
        switch (event.key.keysym.sym) {
            case SDLK_q:      m_key_left  = pressed; break;
            case SDLK_d:      m_key_right = pressed; break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT: m_key_run   = pressed; break;
            case SDLK_SPACE:  if (pressed && !event.key.repeat) m_jump_requested = true; break;
            default: break;
        }
    }

    // Scroll → zoom
    if (event.type == SDL_MOUSEWHEEL && !io.WantCaptureMouse) {
        if (event.wheel.y > 0)      m_camera.zoomBy(ZOOM_STEP);
        else if (event.wheel.y < 0) m_camera.zoomBy(1.0 / ZOOM_STEP);
        m_config.camera.zoom = m_camera.getZoom();
    }

    // Left-click: hand/foot drag takes priority over camera pan
    if (event.type == SDL_MOUSEBUTTONDOWN
        && event.button.button == SDL_BUTTON_LEFT
        && !io.WantCaptureMouse)
    {
        int vw = 0, vh = 0;
        SDL_GetRendererOutputSize(m_renderer, &vw, &vh);
        m_gaze_target_world = m_camera.screenToWorld(
            static_cast<float>(event.button.x),
            static_cast<float>(event.button.y),
            GROUND_Y, vw, vh);

        const SimState& s = m_core->state();
        bool grabbed_body_part = false;
        if (s.character.feet_initialized) {
            const float mx = static_cast<float>(event.button.x);
            const float my = static_cast<float>(event.button.y);
            const SDL_FPoint lh_s = m_camera.worldToScreen(
                s.character.hand_left.x, s.character.hand_left.y,
                GROUND_Y, vw, vh);
            const SDL_FPoint rh_s = m_camera.worldToScreen(
                s.character.hand_right.x, s.character.hand_right.y,
                GROUND_Y, vw, vh);
            const SDL_FPoint lf_s = m_camera.worldToScreen(
                s.character.foot_left.pos.x, s.character.foot_left.pos.y,
                GROUND_Y, vw, vh);
            const SDL_FPoint rf_s = m_camera.worldToScreen(
                s.character.foot_right.pos.x, s.character.foot_right.pos.y,
                GROUND_Y, vw, vh);
            const float dlh = std::sqrt((mx-lh_s.x)*(mx-lh_s.x)+(my-lh_s.y)*(my-lh_s.y));
            const float drh = std::sqrt((mx-rh_s.x)*(mx-rh_s.x)+(my-rh_s.y)*(my-rh_s.y));
            const float dl = std::sqrt((mx-lf_s.x)*(mx-lf_s.x)+(my-lf_s.y)*(my-lf_s.y));
            const float dr = std::sqrt((mx-rf_s.x)*(mx-rf_s.x)+(my-rf_s.y)*(my-rf_s.y));
            float best_d = HAND_HIT_RADIUS_PX;
            enum class DragPart { None, HandLeft, HandRight, FootLeft, FootRight };
            DragPart picked = DragPart::None;
            if (dlh <= best_d) {
                best_d = dlh;
                picked = DragPart::HandLeft;
            }
            if (drh <= best_d) {
                best_d = drh;
                picked = DragPart::HandRight;
            }
            if (dl <= std::min(best_d, FOOT_HIT_RADIUS_PX)) {
                best_d = dl;
                picked = DragPart::FootLeft;
            }
            if (dr <= std::min(best_d, FOOT_HIT_RADIUS_PX)) {
                picked = DragPart::FootRight;
            }

            const Vec2 mouse_world = m_camera.screenToWorld(mx, my, GROUND_Y, vw, vh);
            if (picked == DragPart::HandLeft) {
                m_dragging_hand_left = true;
                m_hand_drag_world = mouse_world;
                grabbed_body_part = true;
            } else if (picked == DragPart::HandRight) {
                m_dragging_hand_right = true;
                m_hand_drag_world = mouse_world;
                grabbed_body_part = true;
            } else if (picked == DragPart::FootLeft) {
                m_dragging_foot_left = true;
                m_foot_drag_world = mouse_world;
                grabbed_body_part = true;
            } else if (picked == DragPart::FootRight) {
                m_dragging_foot_right = true;
                m_foot_drag_world = mouse_world;
                grabbed_body_part = true;
            }
        }
        if (!grabbed_body_part)
            m_is_panning = true;
    }
    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        m_is_panning         = false;
        m_dragging_foot_left  = false;
        m_dragging_foot_right = false;
        m_dragging_hand_left  = false;
        m_dragging_hand_right = false;
    }
    if (event.type == SDL_MOUSEMOTION && !io.WantCaptureMouse) {
        int vw = 0, vh = 0;
        SDL_GetRendererOutputSize(m_renderer, &vw, &vh);
        m_gaze_target_world = m_camera.screenToWorld(
            static_cast<float>(event.motion.x),
            static_cast<float>(event.motion.y),
            GROUND_Y, vw, vh);
        if (m_dragging_foot_left || m_dragging_foot_right) {
            m_foot_drag_world = m_camera.screenToWorld(
                static_cast<float>(event.motion.x),
                static_cast<float>(event.motion.y),
                GROUND_Y, vw, vh);
        } else if (m_dragging_hand_left || m_dragging_hand_right) {
            m_hand_drag_world = m_camera.screenToWorld(
                static_cast<float>(event.motion.x),
                static_cast<float>(event.motion.y),
                GROUND_Y, vw, vh);
        } else if (m_is_panning) {
            const float dx = m_config.camera.follow_x ? 0.f : static_cast<float>(event.motion.xrel);
            const float dy = m_config.camera.follow_y ? 0.f : static_cast<float>(event.motion.yrel);
            if (dx != 0.f || dy != 0.f)
                m_camera.panByScreenDelta(dx, dy);
        }
    }

    // Right-click: pin/unpin hand/foot or drag CM velocity (near CM)
    if (event.type == SDL_MOUSEBUTTONDOWN
        && event.button.button == SDL_BUTTON_RIGHT
        && !io.WantCaptureMouse)
    {
        int vw, vh;
        SDL_GetRendererOutputSize(m_renderer, &vw, &vh);
        const float mx = static_cast<float>(event.button.x);
        const float my = static_cast<float>(event.button.y);
        const SimState& s = m_core->state();

        bool handled_body_part = false;
        if (s.character.feet_initialized) {
            const SDL_FPoint lh_s = m_camera.worldToScreen(
                s.character.hand_left.x, s.character.hand_left.y, GROUND_Y, vw, vh);
            const SDL_FPoint rh_s = m_camera.worldToScreen(
                s.character.hand_right.x, s.character.hand_right.y, GROUND_Y, vw, vh);
            const SDL_FPoint lf_s = m_camera.worldToScreen(
                s.character.foot_left.pos.x, s.character.foot_left.pos.y, GROUND_Y, vw, vh);
            const SDL_FPoint rf_s = m_camera.worldToScreen(
                s.character.foot_right.pos.x, s.character.foot_right.pos.y, GROUND_Y, vw, vh);
            const float dlh = std::sqrt((mx-lh_s.x)*(mx-lh_s.x)+(my-lh_s.y)*(my-lh_s.y));
            const float drh = std::sqrt((mx-rh_s.x)*(mx-rh_s.x)+(my-rh_s.y)*(my-rh_s.y));
            const float dl = std::sqrt((mx-lf_s.x)*(mx-lf_s.x)+(my-lf_s.y)*(my-lf_s.y));
            const float dr = std::sqrt((mx-rf_s.x)*(mx-rf_s.x)+(my-rf_s.y)*(my-rf_s.y));
            float best_d = HAND_HIT_RADIUS_PX;
            enum class HitPart { None, HandLeft, HandRight, FootLeft, FootRight };
            HitPart picked = HitPart::None;
            if (dlh <= best_d) {
                best_d = dlh;
                picked = HitPart::HandLeft;
            }
            if (drh <= best_d) {
                best_d = drh;
                picked = HitPart::HandRight;
            }
            if (dl <= std::min(best_d, FOOT_HIT_RADIUS_PX)) {
                best_d = dl;
                picked = HitPart::FootLeft;
            }
            if (dr <= std::min(best_d, FOOT_HIT_RADIUS_PX)) {
                picked = HitPart::FootRight;
            }

            if (picked == HitPart::HandLeft) {
                m_core->toggleHandPin(true);
                handled_body_part = true;
            } else if (picked == HitPart::HandRight) {
                m_core->toggleHandPin(false);
                handled_body_part = true;
            } else if (picked == HitPart::FootLeft) {
                m_core->toggleFootPin(true);
                handled_body_part = true;
            } else if (picked == HitPart::FootRight) {
                m_core->toggleFootPin(false);
                handled_body_part = true;
            }
        }

        if (!handled_body_part) {
            const Vec2& cm_pos = s.cm.position;
            const SDL_FPoint cm_s = m_camera.worldToScreen(
                cm_pos.x, cm_pos.y, GROUND_Y, vw, vh);
            const float d = std::sqrt((mx-cm_s.x)*(mx-cm_s.x)+(my-cm_s.y)*(my-cm_s.y));
            if (d <= CM_HIT_RADIUS_PX) {
                m_drag_vel_active = true;
                m_drag_mouse_x = mx;
                m_drag_mouse_y = my;
            }
        }
    }
    if (event.type == SDL_MOUSEMOTION && m_drag_vel_active) {
        m_drag_mouse_x = static_cast<float>(event.motion.x);
        m_drag_mouse_y = static_cast<float>(event.motion.y);
    }
    if (event.type == SDL_MOUSEBUTTONUP
        && event.button.button == SDL_BUTTON_RIGHT
        && m_drag_vel_active)
    {
        int vw, vh;
        SDL_GetRendererOutputSize(m_renderer, &vw, &vh);
        const Vec2 mouse_world = m_camera.screenToWorld(
            m_drag_mouse_x, m_drag_mouse_y, GROUND_Y, vw, vh);
        m_pending_set_velocity = mouse_world - m_core->state().cm.position;
        m_drag_vel_active = false;
    }
}

void Application::render()
{
    int vw = 0, vh = 0;
    SDL_GetRendererOutputSize(m_renderer, &vw, &vh);

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    const SimState& s = m_core->state();
    const FrameStats stats{ m_current_fps, m_frame_dt_s };
    const Terrain& terrain = m_core->terrain();
    const AppRequests req = m_debugUI.render(
        stats, m_simLoop, m_config.sim_loop,
        m_camera, m_config.camera,
        m_config.character,
        m_config.head,
        m_config.arms,
        m_config.spline_render,
        m_config.reconstruction,
        s.cm, m_config.cm,
        s.character,
        m_config.standing,
        m_config.physics,
        m_config.terrain,
        m_config.terrain_sampling,
        m_config.walk,
        terrain
    );

    if (req.step_back)          stepBack();
    if (req.regenerate_terrain) m_core->regenerateTerrain();
    if (req.clear_trail)        m_trail.clear();

    // IP test: teleport CM to known initial conditions so DebugUI can compare
    // analytical prediction vs actual physics.
    if (req.ip_test_launch) {
        m_core->teleportCM(req.ip_test_cm_x, req.ip_test_cm_vx);
        SDL_Log("[IPTest] CM teleported to x=%.4f  vx=%.4f",
                req.ip_test_cm_x, req.ip_test_cm_vx);
    }

    if (req.sim_loop || req.camera || req.character || req.head || req.arms || req.spline || req.reconstruction
        || req.walk
        || req.cm || req.physics || req.terrain) {
        m_config.sim_loop.time_scale = m_simLoop.getTimeScale();
        m_config.camera.zoom         = m_camera.getZoom();
        if (ConfigIO::save(CONFIG_PATH, m_config))
            SDL_Log("Config saved → %s", CONFIG_PATH);
        else
            SDL_Log("Failed to save config → %s", CONFIG_PATH);
    }

    ImGui::Render();

    const double ref_h = terrain.height_at(s.cm.position.x);

    SDL_SetRenderDrawColor(m_renderer, 18, 18, 18, 255);
    SDL_RenderClear(m_renderer);

    m_sceneRenderer.render(m_renderer, m_camera, terrain, GROUND_Y, vw, vh);

    m_debugOverlay.renderBackground(m_renderer, m_camera, m_config.cm,
                                    m_trail, m_simLoop.getSimulationTime(),
                                    GROUND_Y, vw, vh);

    m_characterRenderer.render(m_renderer, m_camera, s.cm, s.character,
                               m_config.character, m_config.spline_render,
                               m_config.reconstruction, m_config.cm,
                               terrain, GROUND_Y, vw, vh);

    m_debugOverlay.renderForeground(m_renderer, m_camera, s.cm,
                                    s.character,
                                    m_config.character, m_config.head, m_config.arms, m_config.standing, m_config.cm,
                                    terrain, m_gaze_target_world, ref_h, ACCEL_DISPLAY_SCALE,
                                    m_drag_vel_active, m_drag_mouse_x, m_drag_mouse_y,
                                    GROUND_Y, vw, vh);

    // XCoM (ξ) + step target — shown when show_xcom_line is on and feet exist
    if (m_config.cm.show_xcom_line && s.character.feet_initialized) {
        const bool show_target = std::abs(s.cm.velocity.x) > 0.05;
        m_debugOverlay.renderXCoM(m_renderer, m_camera,
                                   s.xi, s.xi_target_x, s.xi_trigger, show_target,
                                   terrain, GROUND_Y, vw, vh);
    }

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    SDL_RenderPresent(m_renderer);
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
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window   = nullptr; }
    SDL_Quit();
}
