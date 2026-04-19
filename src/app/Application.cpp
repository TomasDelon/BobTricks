#include "app/Application.h"

#include <cmath>
#include <optional>
#include <vector>
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

namespace {

double hash01(int seed)
{
    const double x = std::sin(static_cast<double>(seed) * 12.9898 + 78.233) * 43758.5453123;
    return x - std::floor(x);
}

} // namespace

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

    if (!initAudio())
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

bool Application::initAudio()
{
    SDL_AudioSpec desired{};
    desired.freq = 44100;
    desired.format = AUDIO_F32SYS;
    desired.channels = 1;
    desired.samples = 2048;

    SDL_AudioSpec obtained{};
    m_audio_device = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (m_audio_device == 0)
        return false;

    if (obtained.format != AUDIO_F32SYS || obtained.channels != 1) {
        SDL_Log("Unexpected audio format obtained; closing device");
        SDL_CloseAudioDevice(m_audio_device);
        m_audio_device = 0;
        return false;
    }

    if (!loadFootstepSample()) {
        SDL_CloseAudioDevice(m_audio_device);
        m_audio_device = 0;
        return false;
    }

    SDL_PauseAudioDevice(m_audio_device, 0);
    return true;
}

bool Application::loadFootstepSample()
{
    SDL_AudioSpec wav_spec{};
    Uint8* wav_buffer = nullptr;
    Uint32 wav_length = 0;
    if (SDL_LoadWAV(FOOTSTEP_WAV_PATH, &wav_spec, &wav_buffer, &wav_length) == nullptr) {
        SDL_Log("SDL_LoadWAV failed for %s: %s", FOOTSTEP_WAV_PATH, SDL_GetError());
        return false;
    }

    SDL_AudioCVT cvt{};
    if (SDL_BuildAudioCVT(&cvt,
                          wav_spec.format, wav_spec.channels, wav_spec.freq,
                          AUDIO_F32SYS, 1, 44100) < 0) {
        SDL_Log("SDL_BuildAudioCVT failed: %s", SDL_GetError());
        SDL_FreeWAV(wav_buffer);
        return false;
    }

    cvt.len = static_cast<int>(wav_length);
    cvt.buf = static_cast<Uint8*>(SDL_malloc(static_cast<std::size_t>(cvt.len) * cvt.len_mult));
    if (!cvt.buf) {
        SDL_Log("SDL_malloc failed for audio conversion");
        SDL_FreeWAV(wav_buffer);
        return false;
    }
    SDL_memcpy(cvt.buf, wav_buffer, wav_length);
    SDL_FreeWAV(wav_buffer);

    if (SDL_ConvertAudio(&cvt) < 0) {
        SDL_Log("SDL_ConvertAudio failed: %s", SDL_GetError());
        SDL_free(cvt.buf);
        return false;
    }

    const std::size_t sample_count = static_cast<std::size_t>(cvt.len_cvt) / sizeof(float);
    const float* sample_data = reinterpret_cast<const float*>(cvt.buf);
    m_footstep_sample.assign(sample_data, sample_data + sample_count);
    SDL_free(cvt.buf);
    return !m_footstep_sample.empty();
}

void Application::queueFootstep(float gain)
{
    if (m_audio_device == 0 || m_footstep_sample.empty()) return;

    const float clamped_gain = std::max(0.0f, std::min(gain, 2.0f));
    std::vector<float> scaled = m_footstep_sample;
    for (float& sample : scaled)
        sample *= clamped_gain;

    SDL_QueueAudio(m_audio_device, scaled.data(),
                   static_cast<Uint32>(scaled.size() * sizeof(float)));
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
    m_dust_particles.clear();
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
        const double move_sign = (std::abs(s.cm.velocity.x) > 0.05)
                               ? ((s.cm.velocity.x > 0.0) ? 1.0 : -1.0)
                               : s.character.facing;

        if (s.events.left_touchdown)
            queueFootstep(0.65f);
        if (s.events.right_touchdown)
            queueFootstep(0.65f);
        if (s.events.landed_from_jump)
            queueFootstep(1.0f);

        if (m_config.particles.enabled && m_config.particles.dust_enabled && m_config.particles.impact_enabled) {
            if (s.events.left_touchdown)
                emitFootDust(s.character.foot_left, sim_time, 1.2, move_sign, 0.9);
            if (s.events.right_touchdown)
                emitFootDust(s.character.foot_right, sim_time, 1.2, move_sign, 0.9);
        }

        if (m_config.particles.enabled && m_config.particles.dust_enabled
            && m_config.particles.landing_enabled && s.events.landed_from_jump) {
            const double landing_scale = std::max(1.0, m_config.particles.landing_burst_scale);
            if (s.character.foot_left.on_ground)
                emitFootDust(s.character.foot_left, sim_time, landing_scale, move_sign, 1.3);
            if (s.character.foot_right.on_ground)
                emitFootDust(s.character.foot_right, sim_time, landing_scale, move_sign, 1.3);
        }

        if (m_config.particles.enabled && m_config.particles.dust_enabled) {
            if (m_config.particles.slide_enabled && s.events.left_slide_active) {
                const double uphill_dir = (s.character.foot_left.ground_normal.x >= 0.0) ? -1.0 : 1.0;
                const double motion_dir = (s.cm.velocity.x >= 0.0) ? 1.0 : -1.0;
                const bool braking_on_slope = motion_dir != uphill_dir;
                const double tangent_dir = braking_on_slope ? motion_dir : uphill_dir;
                emitSlideDust(s.character.foot_left, sim_time, tangent_dir);
            }
            if (m_config.particles.slide_enabled && s.events.right_slide_active) {
                const double uphill_dir = (s.character.foot_right.ground_normal.x >= 0.0) ? -1.0 : 1.0;
                const double motion_dir = (s.cm.velocity.x >= 0.0) ? 1.0 : -1.0;
                const bool braking_on_slope = motion_dir != uphill_dir;
                const double tangent_dir = braking_on_slope ? motion_dir : uphill_dir;
                emitSlideDust(s.character.foot_right, sim_time, tangent_dir);
            }
        }
    }
    pruneDustParticles(sim_time);
}

void Application::emitFootDust(const FootState& foot,
                               double sim_time,
                               double burst_scale,
                               double tangent_bias,
                               double vertical_scale)
{
    const int burst_count = std::max(0, static_cast<int>(std::lround(
        static_cast<double>(m_config.particles.dust_burst_count) * burst_scale)));
    if (burst_count == 0 || m_config.particles.dust_lifetime_s <= 0.0) return;

    const Vec2 normal = foot.on_ground ? foot.ground_normal : Vec2{0.0, 1.0};
    Vec2 tangent{normal.y, -normal.x};
    if (tangent.length() < 1.0e-6) tangent = {1.0, 0.0};
    else tangent = tangent / tangent.length();
    const Vec2 up = (normal.length() < 1.0e-6) ? Vec2{0.0, 1.0} : (normal / normal.length());

    const int seed_base = static_cast<int>(sim_time * 1000.0);
    for (int i = 0; i < burst_count; ++i) {
        const double spread = (hash01(seed_base + 31 * i) - 0.5) * 1.1;
        const double forward = 0.55 + 0.75 * hash01(seed_base + 67 * i);
        const double upward = vertical_scale * (0.25 + 0.70 * hash01(seed_base + 101 * i));
        DustParticle particle;
        particle.spawn_time = sim_time;
        particle.lifetime_s = m_config.particles.dust_lifetime_s
                            * (0.75 + 0.5 * hash01(seed_base + 149 * i))
                            * std::max(0.8, burst_scale * 0.8);
        particle.pos = foot.pos + up * 0.015 + tangent * (0.02 * spread);
        particle.vel = tangent * (m_config.particles.dust_speed_mps * (0.45 * spread + 0.55 * tangent_bias) * forward)
                     + up * (m_config.particles.dust_speed_mps * 0.75 * upward);
        particle.radius_px = m_config.particles.dust_radius_px
                           * static_cast<float>((0.7 + 0.8 * hash01(seed_base + 193 * i))
                           * std::max(0.85, burst_scale * 0.75));
        particle.alpha = m_config.particles.dust_alpha
                       * static_cast<float>((0.55 + 0.45 * hash01(seed_base + 239 * i))
                       * std::max(0.85, burst_scale * 0.7));
        m_dust_particles.push_back(particle);
    }
}

void Application::emitSlideDust(const FootState& foot, double sim_time, double tangent_dir)
{
    if (m_config.particles.dust_lifetime_s <= 0.0) return;

    const Vec2 normal = foot.on_ground ? foot.ground_normal : Vec2{0.0, 1.0};
    Vec2 tangent{normal.y, -normal.x};
    if (tangent.length() < 1.0e-6) tangent = {1.0, 0.0};
    else tangent = tangent / tangent.length();
    const Vec2 up = (normal.length() < 1.0e-6) ? Vec2{0.0, 1.0} : (normal / normal.length());

    const int seed_base = static_cast<int>(sim_time * 1000.0) + 7000;
    for (int i = 0; i < 3; ++i) {
        const double jitter = (hash01(seed_base + 17 * i) - 0.5) * 0.6;
        DustParticle particle;
        particle.spawn_time = sim_time;
        particle.lifetime_s = m_config.particles.dust_lifetime_s * (0.45 + 0.15 * hash01(seed_base + 29 * i));
        particle.pos = foot.pos + up * 0.01 + tangent * (0.016 * jitter);
        particle.vel = tangent * (m_config.particles.dust_speed_mps * (0.45 * tangent_dir + 0.25 * jitter))
                     + up * (m_config.particles.dust_speed_mps * (0.10 + 0.15 * hash01(seed_base + 43 * i)));
        particle.radius_px = m_config.particles.dust_radius_px * static_cast<float>(0.45 + 0.20 * hash01(seed_base + 59 * i));
        particle.alpha = m_config.particles.dust_alpha * static_cast<float>(0.35 + 0.15 * hash01(seed_base + 71 * i));
        m_dust_particles.push_back(particle);
    }
}

void Application::pruneDustParticles(double sim_time)
{
    while (!m_dust_particles.empty()) {
        const DustParticle& particle = m_dust_particles.front();
        if (sim_time - particle.spawn_time <= particle.lifetime_s) break;
        m_dust_particles.pop_front();
    }
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

    m_sceneRenderer.render(m_renderer, m_camera, terrain, m_config.particles, m_dust_particles,
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
    if (m_audio_device != 0) {
        SDL_ClearQueuedAudio(m_audio_device);
        SDL_CloseAudioDevice(m_audio_device);
        m_audio_device = 0;
    }
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window   = nullptr; }
    SDL_Quit();
}
