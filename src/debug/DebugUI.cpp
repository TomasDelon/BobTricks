#include "debug/DebugUI.h"
#include "core/locomotion/StandingController.h"
#include "core/physics/Geometry.h"

#include <cmath>
#include <cstdio>
#include "imgui.h"

SaveRequests DebugUI::render(const FrameStats&      stats,
                             SimulationLoop&        simLoop,   SimLoopConfig&   simConfig,
                             Camera2D&              camera,    CameraConfig&    camConfig,
                             CharacterConfig&       charConfig,
                             CharacterReconstructionConfig& reconstructionConfig,
                             const CMState&         cmState,   CMConfig&        cmConfig,
                             const CharacterState&  charState,
                             const StandingConfig&  standConfig,
                             BalanceConfig&         balConfig,
                             PhysicsConfig&         physConfig,
                             TerrainConfig&         terrainConfig)
{
    SaveRequests req;

    ImGui::Begin("Debug");
    renderSimLoopPanel(stats, simLoop, simConfig, req.sim_loop, req.step_back);
    renderCameraPanel(camera, camConfig, req.camera);
    renderTerrainPanel(terrainConfig, req.terrain, req.regenerate_terrain);
    renderPhysicsPanel(physConfig, req.physics);
    renderCharacterPanel(charConfig, standConfig, req.character);
    renderCMKinematicsPanel(cmState);
    renderLocomotionPanel(charState);
    renderBalancePanel(cmState, charState, charConfig, standConfig, balConfig, req.balance);
    renderReconstructionPanel(reconstructionConfig, req.reconstruction);
    renderCMVisualizationPanel(cmConfig, req.cm, req.clear_trail);
    renderIPTestPanel(cmState, charState, charConfig, standConfig, physConfig, simLoop, req);
    ImGui::End();

    return req;
}

// ─── Simulation Loop ─────────────────────────────────────────────────────────

void DebugUI::renderSimLoopPanel(const FrameStats& stats, SimulationLoop& simLoop,
                                  SimLoopConfig& config, bool& saveRequested, bool& stepBack)
{
    if (!ImGui::CollapsingHeader("Simulation Loop", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Text("Current FPS : %.1f", stats.current_fps);

    ImGui::SetNextItemWidth(200.f);
    ImGui::SliderInt("Max FPS", &config.max_fps, 0, 300);

    ImGui::Separator();

    ImGui::Text("Frame dt  : %.4f s  |  Fixed dt : %.4f s",
                stats.frame_dt_s, static_cast<float>(simLoop.getFixedDt()));
    ImGui::Text("Sim step  : %lu",  simLoop.getTotalStepCount());
    ImGui::Text("Sim time  : %.3f s", simLoop.getSimulationTime());

    ImGui::Separator();

    float timeScale = static_cast<float>(simLoop.getTimeScale());
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Time scale", &timeScale, 0.f, 3.f, "%.2fx"))
        simLoop.setTimeScale(static_cast<double>(timeScale));

    if (ImGui::Button("< Step back"))
        stepBack = true;

    ImGui::SameLine();
    if (ImGui::Button(simLoop.isPaused() ? "Resume" : "Pause"))
        simLoop.togglePaused();

    ImGui::SameLine();
    if (ImGui::Button("Step fwd >")) {
        simLoop.setPaused(true);
        simLoop.requestSingleStep();
    }

    ImGui::Separator();
    if (ImGui::Button("Save Simulation Loop Config"))
        saveRequested = true;
}

// ─── Camera ──────────────────────────────────────────────────────────────────

void DebugUI::renderCameraPanel(Camera2D& camera, CameraConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_None))
        return;

    float zoom = static_cast<float>(config.zoom);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Zoom", &zoom, 0.1f, 5.0f, "%.2fx")) {
        config.zoom = static_cast<double>(zoom);
        const double cur = camera.getZoom();
        if (cur > 1e-9) camera.zoomBy(config.zoom / cur);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset##zoom")) {
        camera.resetZoom();
        config.zoom = 1.0;
    }

    ImGui::Text("Scale : %.1f px/m", camera.getPixelsPerMeter());

    ImGui::Separator();

    const Vec2 center = camera.getCenter();
    ImGui::Text("Center x, y : %.3f m , %.3f m", center.x, center.y);

    ImGui::Checkbox("Follow CM.x", &config.follow_x);
    ImGui::SameLine(); ImGui::TextDisabled("(False = drag to pan X)");
    ImGui::Checkbox("Follow CM.y", &config.follow_y);
    ImGui::SameLine(); ImGui::TextDisabled("(False = drag to pan Y)");

    float sx = static_cast<float>(config.smooth_x);
    float sy = static_cast<float>(config.smooth_y);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Smooth X", &sx, 0.f, 5.f, sx <= 0.f ? "instant" : "%.2f s"))
        config.smooth_x = static_cast<double>(sx);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Smooth Y", &sy, 0.f, 5.f, sy <= 0.f ? "instant" : "%.2f s"))
        config.smooth_y = static_cast<double>(sy);

    ImGui::Separator();
    if (ImGui::Button("Save Camera Config"))
        saveRequested = true;
}

// ─── Character Characteristics ───────────────────────────────────────────────

void DebugUI::renderCharacterPanel(CharacterConfig& config, const StandingConfig& standConfig, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Character Characteristics", ImGuiTreeNodeFlags_None))
        return;

    // Height
    float height = static_cast<float>(config.body_height_m);
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Bob height", &height, 1.2f, 2.5f, "%.2f m"))
        config.body_height_m = static_cast<double>(height);
    ImGui::SameLine();
    if (ImGui::Button("Reset##height")) config.body_height_m = 1.80;

    // Mass
    float mass = static_cast<float>(config.body_mass_kg);
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Bob mass", &mass, 40.f, 100.f, "%.1f kg"))
        config.body_mass_kg = static_cast<double>(mass);
    ImGui::SameLine();
    if (ImGui::Button("Reset##mass")) config.body_mass_kg = 60.0;

    // Derived: limb length
    const double L = config.body_height_m / 5.0;
    ImGui::Text("Limb length : H/5 = %.3f m", L);

    ImGui::Separator();

    // CM-to-pelvis ratio
    float ratio = static_cast<float>(config.cm_pelvis_ratio);
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("CM above pelvis", &ratio, 0.60f, 0.85f, "%.2f x L"))
        config.cm_pelvis_ratio = static_cast<double>(ratio);
    ImGui::SameLine();
    if (ImGui::Button("Reset##ratio")) config.cm_pelvis_ratio = 0.75;

    // Derived: nominal CM height (geometry-corrected)
    const double cm_height = computeNominalY(L, standConfig.d_pref, config.cm_pelvis_ratio);
    ImGui::Text("Nominal CM height : %.3f m", cm_height);

    ImGui::Separator();
    if (ImGui::Button("Save Character Config"))
        saveRequested = true;
}

// ─── Character Reconstruction ────────────────────────────────────────────────

void DebugUI::renderReconstructionPanel(CharacterReconstructionConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Character Reconstruction", ImGuiTreeNodeFlags_None))
        return;

    ImGui::TextDisabled("Facing");

    float tau = static_cast<float>(config.facing_tau);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Facing filter (s)", &tau, 0.01f, 0.5f, "%.2f"))
        config.facing_tau = static_cast<double>(tau);

    float feps = static_cast<float>(config.facing_eps);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Facing deadzone (m/s)", &feps, 0.01f, 0.5f, "%.2f"))
        config.facing_eps = static_cast<double>(feps);

    ImGui::Separator();
    ImGui::TextDisabled("Locomotion");

    float weps = static_cast<float>(config.walk_eps);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Walk threshold (m/s)", &weps, 0.01f, 1.0f, "%.2f"))
        config.walk_eps = static_cast<double>(weps);

    ImGui::Separator();
    ImGui::TextDisabled("Pelvis lean");

    float tmax = static_cast<float>(config.theta_max_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Theta max (deg)", &tmax, 0.f, 30.f, "%.1f"))
        config.theta_max_deg = static_cast<double>(tmax);

    float vref = static_cast<float>(config.v_ref);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("V ref (m/s)", &vref, 0.1f, 5.f, "%.2f"))
        config.v_ref = static_cast<double>(vref);

    ImGui::Separator();
    if (ImGui::Button("Save Reconstruction Config"))
        saveRequested = true;
}

// ─── Center of Mass / Locomotion / Visualization ────────────────────────────

void DebugUI::renderCMKinematicsPanel(const CMState& state)
{
    if (!ImGui::CollapsingHeader("Center of Mass", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Text("Position     x: %+.3f m    y: %.3f m",
                state.position.x, state.position.y);
    ImGui::Text("Velocity    vx: %+.3f m/s  vy: %+.3f m/s  v: %.3f m/s",
                state.velocity.x, state.velocity.y, state.velocity.length());
    ImGui::Text("Acceleration ax: %+.3f m/s%c ay: %+.3f m/s%c a: %.3f m/s%c",
                state.acceleration.x, static_cast<char>(178),
                state.acceleration.y, static_cast<char>(178),
                state.acceleration.length(), static_cast<char>(178));
}

void DebugUI::renderLocomotionPanel(const CharacterState& charState)
{
    if (!ImGui::CollapsingHeader("Locomotion", ImGuiTreeNodeFlags_None))
        return;

    const char* locoLabel = "?";
    switch (charState.locomotion_state) {
        case LocomotionState::Standing: locoLabel = "Standing"; break;
        case LocomotionState::Walking:  locoLabel = "Walking";  break;
        case LocomotionState::Airborne: locoLabel = "Airborne"; break;
    }

    ImGui::Text("state      = %s", locoLabel);
    ImGui::Text("facing     = %+.2f", charState.facing);
    ImGui::Text("facing_vel = %+.3f m/s", charState.facing_vel);
}

void DebugUI::renderBalancePanel(const CMState& cmState, const CharacterState& charState,
                                  const CharacterConfig& charConfig, const StandingConfig& standConfig,
                                  BalanceConfig& /*balConfig*/, bool& /*saveRequested*/)
{
    if (!ImGui::CollapsingHeader("Balance", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Separator();

    if (!charState.feet_initialized) {
        ImGui::TextDisabled("(awaiting bootstrap)");
        return;
    }

    const BalanceState& b  = charState.balance;
    const SupportState& s  = charState.support;
    const double L         = charConfig.body_height_m / 5.0;

    ImGui::Text("omega0         = %.3f rad/s", b.omega0);
    ImGui::Text("xi (XCoM)      = %+.4f m",   b.xcom);
    ImGui::Text("support_width  = %.4f m  [%.4f, %.4f]",
                s.width(), standConfig.d_min * L, standConfig.d_max * L);

    // mos in color: green if >= 0, red if < 0
    if (b.mos >= 0.0)
        ImGui::TextColored({0.2f, 0.9f, 0.2f, 1.f}, "mos            = %+.4f m  OK", b.mos);
    else
        ImGui::TextColored({0.9f, 0.2f, 0.2f, 1.f}, "mos            = %+.4f m  !!", b.mos);

    ImGui::Separator();

    const StandingDiag diag = diagStanding(cmState, b.xcom, charState.pelvis, s,
                                           charState.foot_left, charState.foot_right,
                                           charConfig, standConfig);

    // Compact helper: colored tick/cross per criterion
    auto criterion = [](const char* label, bool pass) {
        if (pass)
            ImGui::TextColored({0.2f, 0.9f, 0.2f, 1.f}, "  [OK] %s", label);
        else
            ImGui::TextColored({0.9f, 0.2f, 0.2f, 1.f}, "  [!!] %s", label);
    };

    if (diag.valid())
        ImGui::TextColored({0.2f, 0.9f, 0.2f, 1.f}, "standing_valid = TRUE");
    else
        ImGui::TextColored({0.9f, 0.2f, 0.2f, 1.f}, "standing_valid = FALSE");

    criterion("c1  both planted", diag.c1);

    static constexpr double EPS_C2 = 0.03;
    char c2_buf[80];
    std::snprintf(c2_buf, sizeof(c2_buf), "c2  d=%.6f in [%.4f, %.4f+eps]",
                  diag.d, standConfig.d_min * L, standConfig.d_max * L);
    (void)EPS_C2;  // tolerance applied in diagStanding; shown as "+eps" label
    criterion(c2_buf, diag.c2);

    char c3_buf[64];
    std::snprintf(c3_buf, sizeof(c3_buf), "c3  xcom=%.4f in [%.4f, %.4f]",
                  b.xcom, s.x_left, s.x_right);
    criterion(c3_buf, diag.c3);

    const double max_reach = 2.0 * L;
    char c4_buf[80];
    std::snprintf(c4_buf, sizeof(c4_buf), "c4  rL=%.3f rR=%.3f <= 2L=%.3f",
                  diag.reach_L, diag.reach_R, max_reach);
    criterion(c4_buf, diag.c4);

    char c5_buf[64];
    std::snprintf(c5_buf, sizeof(c5_buf), "c5  |vx|=%.3f <= eps_v=%.3f",
                  std::abs(cmState.velocity.x), standConfig.eps_v);
    criterion(c5_buf, diag.c5);

    // ── Step trigger info ────────────────────────────────────────────────────
    ImGui::Separator();
    if (charState.feet_initialized) {
        const double L      = charConfig.body_height_m / 5.0;
        const double facing = charState.facing;
        const double s_L    = facing * charState.foot_left.pos.x;
        const double s_R    = facing * charState.foot_right.pos.x;
        const bool   rear_R = (s_R <= s_L);
        const double rear_x = rear_R ? charState.foot_right.pos.x
                                     : charState.foot_left.pos.x;
        const double behind = (charState.pelvis.x - rear_x) * facing;
        ImGui::Text("rear foot      = %s", rear_R ? "R" : "L");
        ImGui::Text("behind_dist    = %.3f m   (%.2f L)", behind, behind / L);
        const bool step_active = charState.step_plan.active;
        if (step_active)
            ImGui::TextColored({0.6f, 0.8f, 1.f, 1.f}, "step           = IN FLIGHT (%s)",
                               charState.step_plan.move_right ? "R" : "L");
        else
            ImGui::TextDisabled("step           = idle");
    }
}

void DebugUI::renderCMVisualizationPanel(CMConfig& config, bool& saveRequested, bool& clearTrail)
{
    if (!ImGui::CollapsingHeader("Visualization", ImGuiTreeNodeFlags_None))
        return;

    // Visualization toggles
    ImGui::Checkbox("Ground reference",     &config.show_ground_reference);
    ImGui::Checkbox("Projection line",      &config.show_projection_line);
    ImGui::Checkbox("Projection dot",       &config.show_projection_dot);
    ImGui::Checkbox("Target height tick",   &config.show_target_height_tick);
    ImGui::Checkbox("XCoM line (magenta)",  &config.show_xcom_line);
    ImGui::Checkbox("Support line",         &config.show_support_line);
    ImGui::Checkbox("Planted feet (orange)",&config.show_planted_feet_color);

    ImGui::Separator();

    // Velocity vector components (green)
    ImGui::TextUnformatted("Velocity vector");
    ImGui::SameLine();
    ImGui::RadioButton("Off##vel",  &config.velocity_components, 0); ImGui::SameLine();
    ImGui::RadioButton("X##vel",    &config.velocity_components, 1); ImGui::SameLine();
    ImGui::RadioButton("Y##vel",    &config.velocity_components, 2); ImGui::SameLine();
    ImGui::RadioButton("XY##vel",   &config.velocity_components, 3);

    // Accel vector components (red)
    ImGui::TextUnformatted("Acceleration vector");
    ImGui::SameLine();
    ImGui::RadioButton("Off##acc",  &config.accel_components, 0); ImGui::SameLine();
    ImGui::RadioButton("X##acc",    &config.accel_components, 1); ImGui::SameLine();
    ImGui::RadioButton("Y##acc",    &config.accel_components, 2); ImGui::SameLine();
    ImGui::RadioButton("XY##acc",   &config.accel_components, 3);

    ImGui::Separator();

    // Trail
    ImGui::Checkbox("Trail", &config.show_trail);
    ImGui::BeginDisabled(!config.show_trail);
    float dur = static_cast<float>(config.trail_duration);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Duration (s)", &dur, 0.5f, 15.f, "%.1f s"))
        config.trail_duration = static_cast<double>(dur);
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
        clearTrail = true;
    ImGui::EndDisabled();

    ImGui::Separator();
    if (ImGui::Button("Save CM Config"))
        saveRequested = true;
}

// ─── Physics ─────────────────────────────────────────────────────────────────

void DebugUI::renderPhysicsPanel(PhysicsConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_None))
        return;

    // Gravity
    ImGui::Checkbox("##grav_en", &config.gravity_enabled);
    ImGui::SameLine();
    float g = static_cast<float>(config.gravity);
    ImGui::BeginDisabled(!config.gravity_enabled);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Gravity (m/s²)", &g, 0.f, 20.f, "%.2f"))
        config.gravity = static_cast<double>(g);
    ImGui::EndDisabled();

    // Air friction
    ImGui::Checkbox("##air_en", &config.air_friction_enabled);
    ImGui::SameLine();
    float ka = static_cast<float>(config.air_friction);
    ImGui::BeginDisabled(!config.air_friction_enabled);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Air friction (s⁻¹)", &ka, 0.f, 5.f, "%.2f"))
        config.air_friction = static_cast<double>(ka);
    ImGui::EndDisabled();

    // Floor friction
    ImGui::Checkbox("##floor_en", &config.floor_friction_enabled);
    ImGui::SameLine();
    float kf = static_cast<float>(config.floor_friction);
    ImGui::BeginDisabled(!config.floor_friction_enabled);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Floor friction (s⁻¹)", &kf, 0.f, 20.f, "%.2f"))
        config.floor_friction = static_cast<double>(kf);
    ImGui::EndDisabled();

    ImGui::Separator();

    // Leg spring
    ImGui::Checkbox("##spring_en", &config.spring_enabled);
    ImGui::SameLine();
    ImGui::BeginDisabled(!config.spring_enabled);

    float ks = static_cast<float>(config.spring_stiffness);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Leg stiffness (s⁻²)", &ks, 0.f, 2000.f, "%.0f"))
        config.spring_stiffness = static_cast<double>(ks);

    float kd = static_cast<float>(config.spring_damping);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Leg damping (s⁻¹)", &kd, 0.f, 60.f, "%.1f"))
        config.spring_damping = static_cast<double>(kd);

    ImGui::EndDisabled();

    ImGui::Separator();

    // Locomotion
    float mf = static_cast<float>(config.move_force);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Move force (m/s²)", &mf, 0.f, 40.f, "%.1f"))
        config.move_force = static_cast<double>(mf);

    float ji = static_cast<float>(config.jump_impulse);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Jump impulse (m/s)", &ji, 0.f, 15.f, "%.1f"))
        config.jump_impulse = static_cast<double>(ji);

    ImGui::TextDisabled("Q / D = left / right    SPACE = jump");

    ImGui::Separator();
    if (ImGui::Button("Save Physics Config"))
        saveRequested = true;
}

// ─── Terrain ─────────────────────────────────────────────────────────────────

void DebugUI::renderTerrainPanel(TerrainConfig& config, bool& saveRequested, bool& regenerateRequested)
{
    // saveRequested is re-used; we pass a second bool for regenerate via the panel
    // but SaveRequests already has regenerate_terrain — handled at call site.
    if (!ImGui::CollapsingHeader("Terrain", ImGuiTreeNodeFlags_None))
        return;

    if (ImGui::Checkbox("Enable terrain", &config.enabled))
        regenerateRequested = true;

    ImGui::BeginDisabled(!config.enabled);

    ImGui::Separator();
    ImGui::TextDisabled("Generation");

    ImGui::SetNextItemWidth(120.f);
    ImGui::InputInt("Seed", &config.seed);

    float seg_min = static_cast<float>(config.seg_min);
    float seg_max = static_cast<float>(config.seg_max);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Seg min (m)", &seg_min, 0.5f, 10.f, "%.1f"))
        config.seg_min = static_cast<double>(seg_min);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Seg max (m)", &seg_max, 0.5f, 20.f, "%.1f"))
        config.seg_max = static_cast<double>(seg_max);

    ImGui::Separator();
    ImGui::TextDisabled("Angles");

    float as = static_cast<float>(config.angle_small);
    float al = static_cast<float>(config.angle_large);
    float lp = static_cast<float>(config.large_prob);
    float sm = static_cast<float>(config.slope_max);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Small angle (°)", &as, 0.f, 45.f, "%.1f"))
        config.angle_small = static_cast<double>(as);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Large angle (°)", &al, 0.f, 60.f, "%.1f"))
        config.angle_large = static_cast<double>(al);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Large prob", &lp, 0.f, 1.f, "%.2f"))
        config.large_prob = static_cast<double>(lp);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Slope max (°)", &sm, 5.f, 60.f, "%.1f"))
        config.slope_max = static_cast<double>(sm);

    ImGui::Separator();
    ImGui::TextDisabled("Height bounds");

    float hmin = static_cast<float>(config.height_min);
    float hmax = static_cast<float>(config.height_max);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Height min (m)", &hmin, -10.f, 0.f, "%.1f"))
        config.height_min = static_cast<double>(hmin);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Height max (m)", &hmax, 0.f, 10.f, "%.1f"))
        config.height_max = static_cast<double>(hmax);

    ImGui::EndDisabled();

    ImGui::Separator();
    if (ImGui::Button("Regenerate"))
        regenerateRequested = true;
    ImGui::SameLine();
    if (ImGui::Button("Save Terrain Config"))
        saveRequested = true;
}

// ─── IP Completion Test ───────────────────────────────────────────────────────

void DebugUI::renderIPTestPanel(const CMState&        cmState,
                                const CharacterState& charState,
                                const CharacterConfig& charConfig,
                                const StandingConfig& standConfig,
                                const PhysicsConfig&  physConfig,
                                SimulationLoop&       simLoop,
                                SaveRequests&         req)
{
    if (!ImGui::CollapsingHeader("IP Completion Test"))
        return;

    // ── Derived constants ────────────────────────────────────────────────────
    const double L         = charConfig.body_height_m / 5.0;
    const double nominal_y = computeNominalY(L, standConfig.d_pref, charConfig.cm_pelvis_ratio);
    const double omega0    = std::sqrt(physConfig.gravity / nominal_y);
    const double mu        = physConfig.floor_friction;

    ImGui::TextDisabled("Tests: a_x = (g/h)*offset drives CM after key release.");
    ImGui::TextDisabled("Condition: step active, vx=0, CM placed at stance+0.3L");
    ImGui::Separator();

    // ── Launch button ────────────────────────────────────────────────────────
    const bool step_active = charState.step_plan.active;

    if (!step_active)
        ImGui::BeginDisabled();

    if (ImGui::Button("Launch  [CM = stance + 0.3L,  vx = 0]") && step_active) {
        const StepPlan& plan = charState.step_plan;
        const FootState& stance_foot = plan.move_right ? charState.foot_left
                                                       : charState.foot_right;
        const double stance_x  = stance_foot.pos.x;
        const double facing    = charState.facing;
        const double x0_rel    = 0.3 * L;          // offset from stance foot
        const double target_x  = stance_x + facing * x0_rel;

        // Analytical solution of  x'' = ω₀²·x − μ·x'
        // Char. eq: r² + μ·r − ω₀² = 0
        // (x here is signed offset in facing direction, so ω₀² acts as destabilising)
        const double disc = std::sqrt(mu*mu + 4.0*omega0*omega0);
        const double r1   = (-mu + disc) * 0.5;   // positive (unstable) root
        const double r2   = (-mu - disc) * 0.5;   // negative (stable)   root
        const double v0   = 0.0;
        const double C1   = (v0 - r2*x0_rel) / (r1 - r2);
        const double C2   = (r1*x0_rel - v0) / (r1 - r2);

        m_ipTest = { true,
                     simLoop.getSimulationTime(),
                     stance_x, facing,
                     x0_rel, v0,
                     omega0, mu, r1, r2, C1, C2 };

        // Signal Application to teleport CM.
        req.ip_test_launch = true;
        req.ip_test_cm_x   = target_x;
        req.ip_test_cm_vx  = 0.0;
    }
    if (!step_active) {
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextColored({1,0.4f,0,1}, "No step active — walk first");
    }

    if (m_ipTest.active && ImGui::Button("Stop test"))
        m_ipTest.active = false;

    if (!m_ipTest.active) return;

    ImGui::Separator();

    // ── Running test display ─────────────────────────────────────────────────
    const double t = simLoop.getSimulationTime() - m_ipTest.t_start;

    // Analytical prediction
    const double exp1      = std::exp(m_ipTest.r1 * t);
    const double exp2      = std::exp(m_ipTest.r2 * t);
    const double x_pred    = m_ipTest.C1 * exp1 + m_ipTest.C2 * exp2;
    const double v_pred    = m_ipTest.C1 * m_ipTest.r1 * exp1
                           + m_ipTest.C2 * m_ipTest.r2 * exp2;

    // Actual
    const double x_rel_act = (cmState.position.x - m_ipTest.stance_x) * m_ipTest.facing;
    const double v_act     = cmState.velocity.x * m_ipTest.facing;

    const double dx_pred   = x_pred - m_ipTest.x0_rel;
    const double dx_act    = x_rel_act - m_ipTest.x0_rel;

    // "Null" (no IP force): vx=0, no driving force → stays at x0_rel
    const double x_null    = m_ipTest.x0_rel;

    ImGui::Text("t = %.3f s", t);
    ImGui::Text("omega0 = %.3f rad/s   mu (friction) = %.2f s^-1", omega0, mu);
    ImGui::Text("r1 = %+.3f  r2 = %+.3f", m_ipTest.r1, m_ipTest.r2);
    ImGui::Separator();

    ImGui::TextColored({0.4f,1,0.4f,1}, "Analytical  (IP + friction):");
    ImGui::Text("  x_pred = stance %+.4f m   delta = %+.4f m", x_pred, dx_pred);
    ImGui::Text("  v_pred =        %+.4f m/s", v_pred);

    ImGui::TextColored({1,0.8f,0.2f,1}, "Actual:");
    ImGui::Text("  x_act  = stance %+.4f m   delta = %+.4f m", x_rel_act, dx_act);
    ImGui::Text("  v_act  =        %+.4f m/s", v_act);

    const double err_x = x_rel_act - x_pred;
    const double err_v = v_act - v_pred;
    ImGui::Text("  error  x = %+.4f m   v = %+.4f m/s", err_x, err_v);
    ImGui::Separator();

    ImGui::TextColored({0.5f,0.5f,0.5f,1}, "Without IP force (vx=0, no drive):");
    ImGui::Text("  x_null = stance %+.4f m   delta =  0.0000 m  <-- would stay", x_null);
    ImGui::Text("  v_null =         0.0000 m/s");
    ImGui::Separator();

    // Result verdict
    if (dx_act > 0.001) {
        ImGui::TextColored({0.2f,1,0.2f,1},
            "PASS  IP force moving CM forward  (delta_x = %+.4f m)", dx_act);
    } else if (dx_act > -0.001) {
        ImGui::TextColored({1,1,0.2f,1}, "WAIT  CM not yet moving...");
    } else {
        ImGui::TextColored({1,0.2f,0.2f,1}, "FAIL  CM moving backward!");
    }
}
