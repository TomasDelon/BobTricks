#include "debug/DebugUI.h"
#include "core/locomotion/StandingController.h"
#include "core/physics/Geometry.h"

#include <cmath>
#include <cstdio>
#include "imgui.h"

namespace {

const char* locomotionStateLabel(LocomotionState state)
{
    switch (state) {
        case LocomotionState::Standing: return "Standing";
        case LocomotionState::Walking:  return "Walking";
        case LocomotionState::Running:  return "Running";
        case LocomotionState::Airborne: return "Airborne";
    }
    return "?";
}

void renderLocomotionPoseDebug(const CharacterState&  charState,
                               const CharacterConfig& charConfig,
                               const Terrain&         terrain)
{
    ImGui::Text("state      = %s", locomotionStateLabel(charState.locomotion_state));
    ImGui::Text("facing     = %+.2f", charState.facing);
    ImGui::Text("on_floor   = %s", charState.debug_on_floor ? "true" : "false");
    ImGui::Text("cm_target_y = %+.3f m", charState.debug_cm_target_y);

    const double L        = charConfig.body_height_m / 5.0;
    const double ground_y = terrain.height_at(charState.pelvis.x);
    const double pelvis_h = charState.pelvis.y - ground_y;
    ImGui::Text("pelvis_h    = %.3f m  (%.2f L)", pelvis_h, pelvis_h / L);
    ImGui::Text("theta       = %+.2f deg  slope_lp = %+.3f",
                charState.theta * 180.0 / 3.14159265358979323846,
                charState.filtered_slope);
}

void renderFootDebug(const char* label, const FootState& foot)
{
    ImGui::Separator();
    ImGui::TextDisabled("%s foot", label);
    ImGui::Text("pos        = (%+.4f, %+.4f)", foot.pos.x, foot.pos.y);
    if (foot.on_ground)
        ImGui::TextColored({0.2f, 0.9f, 0.2f, 1.f}, "on_ground  = true");
    else
        ImGui::TextColored({0.9f, 0.5f, 0.1f, 1.f}, "on_ground  = false");
    ImGui::Text("normal     = (%.3f, %.3f)", foot.ground_normal.x, foot.ground_normal.y);
}

void renderWalkParams(WalkConfig& walkConfig)
{
    ImGui::Separator();
    ImGui::TextDisabled("Walk params");
    {
        float xs = static_cast<float>(walkConfig.xcom_scale);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("xcom_scale", &xs, 0.0f, 1.0f, "%.2f"))
            walkConfig.xcom_scale = static_cast<double>(xs);
        ImGui::SameLine(); ImGui::TextDisabled("alpha*v/w0 in xi");
    }
    {
        float dr = static_cast<float>(walkConfig.d_rear_max);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("d_rear_max (xL)", &dr, 0.5f, 3.0f, "%.2f"))
            walkConfig.d_rear_max = static_cast<double>(dr);
        ImGui::SameLine(); ImGui::TextDisabled("rear rescue trigger");
    }
    {
        float ms = static_cast<float>(walkConfig.max_step_L);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("max_step_L (xL)", &ms, 0.5f, 4.0f, "%.2f"))
            walkConfig.max_step_L = static_cast<double>(ms);
        ImGui::SameLine(); ImGui::TextDisabled("max step from stance foot");
    }
    {
        float sm = static_cast<float>(walkConfig.stability_margin);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("stability_margin (xL)", &sm, 0.0f, 1.5f, "%.2f"))
            walkConfig.stability_margin = static_cast<double>(sm);
    }
    {
        float ss = static_cast<float>(walkConfig.step_speed);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("step_speed (steps/s)", &ss, 0.5f, 15.0f, "%.1f"))
            walkConfig.step_speed = static_cast<double>(ss);
    }
    {
        float cho = static_cast<float>(walkConfig.cm_height_offset);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("cm_height_offset (m)", &cho, -0.3f, 0.3f, "%.3f"))
            walkConfig.cm_height_offset = static_cast<double>(cho);
    }
    {
        float dst = static_cast<float>(walkConfig.double_support_time);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("double_support_time (s)", &dst, 0.0f, 0.15f, "%.3f"))
            walkConfig.double_support_time = static_cast<double>(dst);
        ImGui::SameLine(); ImGui::TextDisabled("min both-feet time after heel-strike");
    }
    ImGui::Separator();
    ImGui::TextDisabled("IP bob");
    {
        float lf = static_cast<float>(walkConfig.leg_flex_coeff);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("leg_flex_coeff (xL)", &lf, 0.0f, 0.3f, "%.3f"))
            walkConfig.leg_flex_coeff = static_cast<double>(lf);
        ImGui::SameLine(); ImGui::TextDisabled("knee bend at mid-stance");
    }
    {
        float bs = static_cast<float>(walkConfig.bob_scale);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("bob_scale", &bs, 0.0f, 10.0f, "%.2f"))
            walkConfig.bob_scale = static_cast<double>(bs);
        ImGui::SameLine(); ImGui::TextDisabled("arc deviation multiplier");
    }
    {
        float ba = static_cast<float>(walkConfig.bob_amp);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("bob_amp (xL)", &ba, 0.0f, 0.5f, "%.3f"))
            walkConfig.bob_amp = static_cast<double>(ba);
        ImGui::SameLine(); ImGui::TextDisabled("max drop cap");
    }
    ImGui::Separator();
    ImGui::TextDisabled("Foot lift (h_clear)");
    {
        float sf = static_cast<float>(walkConfig.h_clear_slope_factor);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("h_clear_slope (xL/slope)", &sf, 0.0f, 2.0f, "%.2f"))
            walkConfig.h_clear_slope_factor = static_cast<double>(sf);
        ImGui::SameLine(); ImGui::TextDisabled("extra lift per unit uphill slope");
    }
    {
        float spf = static_cast<float>(walkConfig.h_clear_speed_factor);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("h_clear_speed (xL)", &spf, 0.0f, 0.5f, "%.3f"))
            walkConfig.h_clear_speed_factor = static_cast<double>(spf);
        ImGui::SameLine(); ImGui::TextDisabled("extra lift at walk_max_speed");
    }
    {
        float mn = static_cast<float>(walkConfig.h_clear_min_ratio);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("h_clear_min (xL)", &mn, 0.0f, 0.2f, "%.3f"))
            walkConfig.h_clear_min_ratio = static_cast<double>(mn);
        ImGui::SameLine(); ImGui::TextDisabled("minimum foot lift (anti-drag)");
    }
}

void renderPhysicsGravityControls(PhysicsConfig& config)
{
    ImGui::Checkbox("##grav_en", &config.gravity_enabled);
    ImGui::SameLine();
    float g = static_cast<float>(config.gravity);
    ImGui::BeginDisabled(!config.gravity_enabled);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Gravity (m/s²)", &g, 0.f, 20.f, "%.2f"))
        config.gravity = static_cast<double>(g);
    ImGui::EndDisabled();
}

void renderPhysicsVerticalTrackingControls(PhysicsConfig& config)
{
    ImGui::Checkbox("##spring_en", &config.spring_enabled);
    ImGui::SameLine();
    ImGui::BeginDisabled(!config.spring_enabled);

    float vy_max = static_cast<float>(config.vy_max);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("vy_max (m/s)", &vy_max, 0.1f, 8.0f, "%.2f"))
        config.vy_max = static_cast<double>(vy_max);

    float d_soft = static_cast<float>(config.d_soft);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("d_soft (m)", &d_soft, 0.01f, 1.0f, "%.3f"))
        config.d_soft = static_cast<double>(d_soft);

    float vy_tau = static_cast<float>(config.vy_tau);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("vy_tau (s⁻¹)", &vy_tau, 1.0f, 60.0f, "%.1f"))
        config.vy_tau = static_cast<double>(vy_tau);

    ImGui::EndDisabled();
}

void renderPhysicsLocomotionControls(PhysicsConfig& config)
{
    float ac = static_cast<float>(config.accel);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Accel (m/s²)", &ac, 0.f, 20.f, "%.1f"))
        config.accel = static_cast<double>(ac);

    float ms = static_cast<float>(config.walk_max_speed);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Max speed (m/s)", &ms, 0.1f, 6.f, "%.2f"))
        config.walk_max_speed = static_cast<double>(ms);

    float hs = static_cast<float>(config.hold_speed);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Hold speed (m/s)", &hs, 0.0f, 2.f, "%.2f"))
        config.hold_speed = static_cast<double>(hs);

    float ji = static_cast<float>(config.jump_impulse);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Jump impulse (m/s)", &ji, 0.f, 15.f, "%.1f"))
        config.jump_impulse = static_cast<double>(ji);
}

void renderTerrainGenerationControls(TerrainConfig& config)
{
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
}

void renderTerrainAngleControls(TerrainConfig& config)
{
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
}

void renderTerrainHeightControls(TerrainConfig& config)
{
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
}

void renderTerrainSamplingControls(TerrainSamplingConfig& config)
{
    float wb = static_cast<float>(config.w_back);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("w_back (xL)", &wb, 0.1f, 2.0f, "%.2f"))
        config.w_back = static_cast<double>(wb);

    float wf = static_cast<float>(config.w_fwd);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("w_fwd (xL)", &wf, 0.1f, 2.0f, "%.2f"))
        config.w_fwd = static_cast<double>(wf);

    float tl = static_cast<float>(config.t_look);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("t_look (s)", &tl, 0.0f, 1.0f, "%.2f"))
        config.t_look = static_cast<double>(tl);

    float ts = static_cast<float>(config.tau_slide);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("tau_slide (s)", &ts, 0.01f, 1.0f, "%.3f"))
        config.tau_slide = static_cast<double>(ts);
    ImGui::SameLine(); ImGui::TextDisabled("endpoint slide lag");
}

void renderCharacterBodyControls(CharacterConfig& config)
{
    float height = static_cast<float>(config.body_height_m);
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Bob height", &height, 1.2f, 2.5f, "%.2f m"))
        config.body_height_m = static_cast<double>(height);
    ImGui::SameLine();
    if (ImGui::Button("Reset##height")) config.body_height_m = 1.80;
}

void renderCharacterPelvisControls(CharacterConfig& config)
{
    float ratio = static_cast<float>(config.cm_pelvis_ratio);
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("CM above pelvis", &ratio, 0.60f, 0.85f, "%.2f x L"))
        config.cm_pelvis_ratio = static_cast<double>(ratio);
    ImGui::SameLine();
    if (ImGui::Button("Reset##ratio")) config.cm_pelvis_ratio = 0.75;

    ImGui::Checkbox("Show pelvis reach disk (2L)", &config.show_pelvis_reach_disk);
}

void renderReconstructionFacingControls(CharacterReconstructionConfig& config)
{
    ImGui::TextDisabled("Facing");

    float feps = static_cast<float>(config.facing_eps);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Facing deadzone (m/s)", &feps, 0.01f, 0.5f, "%.2f"))
        config.facing_eps = static_cast<double>(feps);
}

void renderReconstructionLocomotionControls(CharacterReconstructionConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Locomotion");

    float weps = static_cast<float>(config.walk_eps);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Walk threshold (m/s)", &weps, 0.01f, 1.0f, "%.2f"))
        config.walk_eps = static_cast<double>(weps);
}

void renderReconstructionLeanControls(CharacterReconstructionConfig& config)
{
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

    float hunch_min = static_cast<float>(config.hunch_min_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Hunch min (deg)", &hunch_min, 0.f, 20.f, "%.1f"))
        config.hunch_min_deg = static_cast<double>(hunch_min);

    float hunch_max = static_cast<float>(config.hunch_max_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Hunch max (deg)", &hunch_max, 0.f, 20.f, "%.1f"))
        config.hunch_max_deg = static_cast<double>(hunch_max);

    float hunch_current = static_cast<float>(config.hunch_current_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Hunch current (deg)", &hunch_current, 0.f, 20.f, "%.1f"))
        config.hunch_current_deg = static_cast<double>(hunch_current);
}

void renderBalanceReachMetrics(const CharacterState& charState, double L)
{
    const auto& lf = charState.foot_left;
    const auto& rf = charState.foot_right;
    const double dx_l = lf.pos.x - charState.pelvis.x;
    const double dy_l = lf.pos.y - charState.pelvis.y;
    const double dx_r = rf.pos.x - charState.pelvis.x;
    const double dy_r = rf.pos.y - charState.pelvis.y;
    const double reach_L = std::sqrt(dx_l * dx_l + dy_l * dy_l);
    const double reach_R = std::sqrt(dx_r * dx_r + dy_r * dy_r);

    ImGui::Text("reach_L = %.3f m  (%.2f L)", reach_L, reach_L / L);
    ImGui::Text("reach_R = %.3f m  (%.2f L)", reach_R, reach_R / L);
    ImGui::Text("max 2L  = %.3f m", 2.0 * L);

    if (reach_L > 2.0 * L - 0.01)
        ImGui::TextColored({0.9f, 0.3f, 0.1f, 1.f}, "L foot at circle limit!");
    if (reach_R > 2.0 * L - 0.01)
        ImGui::TextColored({0.9f, 0.3f, 0.1f, 1.f}, "R foot at circle limit!");

    ImGui::Separator();
    const double foot_sep = std::abs(rf.pos.x - lf.pos.x);
    ImGui::Text("foot separation = %.3f m  (%.2f L)", foot_sep, foot_sep / L);
}

void renderVisualizationVectorControls(CMConfig& config)
{
    ImGui::TextUnformatted("Velocity vector");
    ImGui::SameLine();
    ImGui::RadioButton("Off##vel",  &config.velocity_components, 0); ImGui::SameLine();
    ImGui::RadioButton("X##vel",    &config.velocity_components, 1); ImGui::SameLine();
    ImGui::RadioButton("Y##vel",    &config.velocity_components, 2); ImGui::SameLine();
    ImGui::RadioButton("XY##vel",   &config.velocity_components, 3);

    ImGui::TextUnformatted("Acceleration vector");
    ImGui::SameLine();
    ImGui::RadioButton("Off##acc",  &config.accel_components, 0); ImGui::SameLine();
    ImGui::RadioButton("X##acc",    &config.accel_components, 1); ImGui::SameLine();
    ImGui::RadioButton("Y##acc",    &config.accel_components, 2); ImGui::SameLine();
    ImGui::RadioButton("XY##acc",   &config.accel_components, 3);
}

void renderVisualizationTrailControls(CMConfig& config, bool& clearTrail)
{
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
}

void renderArmReachControls(ArmConfig& config)
{
    float upper = static_cast<float>(config.upper_arm_L);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("upper_arm_L (xL)", &upper, 0.5f, 1.5f, "%.2f"))
        config.upper_arm_L = static_cast<double>(upper);

    float fore = static_cast<float>(config.fore_arm_L);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("fore_arm_L (xL)", &fore, 0.5f, 1.5f, "%.2f"))
        config.fore_arm_L = static_cast<double>(fore);

    float retract = static_cast<float>(config.walk_hand_reach_reduction_L);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("walk_hand_reach_reduction_L (xL)", &retract, 0.0f, 1.25f, "%.2f"))
        config.walk_hand_reach_reduction_L = static_cast<double>(retract);

    float phase_scale = static_cast<float>(config.walk_hand_phase_speed_scale);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("walk_hand_phase_speed_scale", &phase_scale, 0.1f, 2.0f, "%.2f"))
        config.walk_hand_phase_speed_scale = static_cast<double>(phase_scale);

    float speed_arc_gain = static_cast<float>(config.walk_hand_speed_arc_gain);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("walk_hand_speed_arc_gain", &speed_arc_gain, 0.0f, 1.0f, "%.2f"))
        config.walk_hand_speed_arc_gain = static_cast<double>(speed_arc_gain);

    float phase_response = static_cast<float>(config.walk_hand_phase_response);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("walk_hand_phase_response", &phase_response, 1.0f, 30.0f, "%.1f"))
        config.walk_hand_phase_response = static_cast<double>(phase_response);

    float phase_friction = static_cast<float>(config.walk_hand_phase_friction);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("walk_hand_phase_friction", &phase_friction, 0.1f, 12.0f, "%.1f"))
        config.walk_hand_phase_friction = static_cast<double>(phase_friction);
}

void renderArmArcControls(ArmConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Front hand arc");

    float front_start = static_cast<float>(config.walk_front_hand_start_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("front_start_deg", &front_start, -180.f, 180.f, "%.1f"))
        config.walk_front_hand_start_deg = static_cast<double>(front_start);

    float front_end = static_cast<float>(config.walk_front_hand_end_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("front_end_deg", &front_end, -180.f, 180.f, "%.1f"))
        config.walk_front_hand_end_deg = static_cast<double>(front_end);

    ImGui::Separator();
    ImGui::TextDisabled("Back hand arc");

    float back_start = static_cast<float>(config.walk_back_hand_start_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("back_start_deg", &back_start, -180.f, 180.f, "%.1f"))
        config.walk_back_hand_start_deg = static_cast<double>(back_start);

    float back_end = static_cast<float>(config.walk_back_hand_end_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("back_end_deg", &back_end, -180.f, 180.f, "%.1f"))
        config.walk_back_hand_end_deg = static_cast<double>(back_end);
}

void renderArmOverlayControls(ArmConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Overlay");
    ImGui::Checkbox("Show reach circles", &config.show_debug_reach_circles);
    ImGui::Checkbox("Show swing points", &config.show_debug_swing_points);
    ImGui::Checkbox("Show swing arcs", &config.show_debug_swing_arcs);
}

void renderSplineControls(SplineRenderConfig& config)
{
    ImGui::Checkbox("Use spline renderer", &config.enabled);
    ImGui::Checkbox("Draw spline under legacy", &config.draw_under_legacy);
    ImGui::Checkbox("Render head", &config.show_head);
    ImGui::Checkbox("Render torso", &config.show_torso);
    ImGui::Checkbox("Render arms", &config.show_arms);
    ImGui::Checkbox("Render legs", &config.show_legs);
    ImGui::Checkbox("Show test curve", &config.show_test_curve);
    ImGui::Checkbox("Show control polygon", &config.show_control_polygon);
    ImGui::Checkbox("Show sample points", &config.show_sample_points);

    ImGui::Separator();

    ImGui::SetNextItemWidth(180.f);
    ImGui::SliderFloat("stroke_width_px", &config.stroke_width_px, 1.0f, 32.0f, "%.1f");

    ImGui::SetNextItemWidth(180.f);
    ImGui::SliderInt("samples_per_curve", &config.samples_per_curve, 4, 96);
}

} // namespace

AppRequests DebugUI::render(const FrameStats&      stats,
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
                             TerrainSamplingConfig& terrainSamplingConfig,
                             WalkConfig&            walkConfig,
                             JumpConfig&            jumpConfig,
                             const Terrain&         terrain)
{
    AppRequests req;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 work_pos = viewport->WorkPos;
    const ImVec2 work_size = viewport->WorkSize;
    const ImVec2 margin = {8.0f, 8.0f};
    const float debug_width = std::min(401.0f, std::max(301.0f, work_size.x * 0.34f - 19.0f));
    const float debug_height = std::max(322.0f, work_size.y - 2.0f * margin.y - 38.0f);
    ImGui::SetNextWindowPos({work_pos.x + margin.x, work_pos.y + margin.y}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({debug_width, debug_height}, ImGuiCond_FirstUseEver);

    ImGui::Begin("Debug");
    renderSimLoopPanel(stats, simLoop, simConfig, req.sim_loop, req.step_back);
    renderCameraPanel(camera, camConfig, req.camera);
    renderTerrainPanel(terrainConfig, req.terrain, req.regenerate_terrain);
    renderCMKinematicsPanel(cmState, cmConfig, req.cm, req.clear_trail);
    renderTerrainSamplingPanel(terrainSamplingConfig, cmConfig, req.terrain);
    renderCharacterPanel(charConfig, standConfig, req.character);
    renderHeadPanel(charState, headConfig, req.head);
    renderReconstructionPanel(reconstructionConfig, req.reconstruction);
    renderSplinePanel(splineConfig, req.spline);
    renderTorsoPanel(charState, charConfig, reconstructionConfig, terrain, req.reconstruction);
    renderLegsPanel(charState, charConfig, cmConfig, req.character);
    renderLocomotionPanel(cmState, charState, charConfig, reconstructionConfig,
                          terrain, walkConfig, req.walk);
    renderBalancePanel(cmState, charState, charConfig, standConfig, cmConfig, req.cm);
    renderArmsPanel(armConfig, req.arms);
    renderJumpPanel(jumpConfig, req.jump);
    renderPhysicsPanel(physConfig, req.physics);
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
    float dzx = static_cast<float>(config.deadzone_x);
    float dzy = static_cast<float>(config.deadzone_y);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Smooth X", &sx, 0.f, 5.f, sx <= 0.f ? "instant" : "%.2f s"))
        config.smooth_x = static_cast<double>(sx);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Smooth Y", &sy, 0.f, 5.f, sy <= 0.f ? "instant" : "%.2f s"))
        config.smooth_y = static_cast<double>(sy);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Deadzone X", &dzx, 0.f, 2.f, "%.2f m"))
        config.deadzone_x = static_cast<double>(dzx);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Deadzone Y", &dzy, 0.f, 2.f, "%.2f m"))
        config.deadzone_y = static_cast<double>(dzy);

    ImGui::Separator();
    if (ImGui::Button("Save Camera Config"))
        saveRequested = true;
}

// ─── Character Characteristics ───────────────────────────────────────────────

void DebugUI::renderCharacterPanel(CharacterConfig& config, const StandingConfig& standConfig, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Character Characteristics", ImGuiTreeNodeFlags_None))
        return;

    renderCharacterBodyControls(config);

    // Derived: limb length
    const double L = config.body_height_m / 5.0;
    ImGui::Text("Limb length : H/5 = %.3f m", L);

    ImGui::Separator();

    renderCharacterPelvisControls(config);

    // Derived: nominal CM height (geometry-corrected)
    const double cm_height = computeNominalY(L, standConfig.d_pref, config.cm_pelvis_ratio);
    ImGui::Text("Nominal CM height : %.3f m", cm_height);

    ImGui::Separator();
    if (ImGui::Button("Save Character Config"))
        saveRequested = true;
}

void DebugUI::renderArmsPanel(ArmConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Arms", ImGuiTreeNodeFlags_None))
        return;

    renderArmReachControls(config);
    renderArmArcControls(config);
    renderArmOverlayControls(config);

    ImGui::Separator();
    if (ImGui::Button("Save Arms Config"))
        saveRequested = true;
}

void DebugUI::renderHeadPanel(const CharacterState& charState, HeadConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Head", ImGuiTreeNodeFlags_None))
        return;

    ImGui::TextDisabled("Geometry");
    ImGui::Text("head_center = (%+.3f, %+.3f)", charState.head_center.x, charState.head_center.y);
    ImGui::Text("head_radius = %.3f m", charState.head_radius);
    ImGui::Text("head_tilt   = %+.2f deg", charState.head_tilt * 180.0 / 3.14159265358979323846);

    float offset = static_cast<float>(config.center_offset_L);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("center_offset_L", &offset, 0.1f, 1.0f, "%.2f"))
        config.center_offset_L = static_cast<double>(offset);

    float radius = static_cast<float>(config.radius_L);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("radius_L", &radius, 0.1f, 1.0f, "%.2f"))
        config.radius_L = static_cast<double>(radius);

    ImGui::Separator();
    ImGui::TextDisabled("Gaze / Intent");
    float tilt = static_cast<float>(config.max_tilt_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("max_tilt_deg", &tilt, 0.f, 60.f, "%.1f"))
        config.max_tilt_deg = static_cast<double>(tilt);

    float tau = static_cast<float>(config.tau_tilt);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("tau_tilt", &tau, 0.01f, 1.0f, "%.2f"))
        config.tau_tilt = static_cast<double>(tau);

    ImGui::Checkbox("Show eye marker", &config.show_eye_marker);
    ImGui::Checkbox("Show gaze ray", &config.show_gaze_ray);
    ImGui::Checkbox("Show gaze target", &config.show_gaze_target);

    ImGui::Separator();
    if (ImGui::Button("Save Head Config"))
        saveRequested = true;
}

void DebugUI::renderSplinePanel(SplineRenderConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Spline Rendering", ImGuiTreeNodeFlags_None))
        return;

    renderSplineControls(config);

    ImGui::Separator();
    if (ImGui::Button("Save Spline Config"))
        saveRequested = true;
}

// ─── Character Reconstruction ────────────────────────────────────────────────

void DebugUI::renderReconstructionPanel(CharacterReconstructionConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Character Reconstruction", ImGuiTreeNodeFlags_None))
        return;

    renderReconstructionFacingControls(config);
    renderReconstructionLocomotionControls(config);

    ImGui::Separator();
    if (ImGui::Button("Save Reconstruction Config"))
        saveRequested = true;
}

// ─── Center of Mass / Torso / Legs / Locomotion / Balance ───────────────────

void DebugUI::renderCMKinematicsPanel(const CMState& state, CMConfig& config, bool& saveRequested, bool& clearTrail)
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

    ImGui::Separator();
    ImGui::Checkbox("Projection line",    &config.show_projection_line);
    ImGui::Checkbox("Projection dot",     &config.show_projection_dot);
    ImGui::Checkbox("Target height tick", &config.show_target_height_tick);

    ImGui::Separator();
    renderVisualizationVectorControls(config);

    ImGui::Separator();
    renderVisualizationTrailControls(config, clearTrail);

    ImGui::Separator();
    if (ImGui::Button("Save CM Config"))
        saveRequested = true;
}

void DebugUI::renderTorsoPanel(const CharacterState& charState,
                               const CharacterConfig& charConfig,
                               CharacterReconstructionConfig& reconstructionConfig,
                               const Terrain& terrain,
                               bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Torso", ImGuiTreeNodeFlags_None))
        return;

    renderLocomotionPoseDebug(charState, charConfig, terrain);

    ImGui::Separator();
    renderReconstructionLeanControls(reconstructionConfig);

    ImGui::Separator();
    if (ImGui::Button("Save Torso Config"))
        saveRequested = true;
}

void DebugUI::renderLegsPanel(const CharacterState& charState,
                              CharacterConfig& charConfig,
                              CMConfig& /*cmConfig*/,
                              bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Legs", ImGuiTreeNodeFlags_None))
        return;

    if (!charState.feet_initialized) {
        ImGui::TextDisabled("(awaiting bootstrap)");
    } else {
        renderFootDebug("Left",  charState.foot_left);
        renderFootDebug("Right", charState.foot_right);
    }

    ImGui::Separator();
    ImGui::Checkbox("Show pelvis reach disk (2L)", &charConfig.show_pelvis_reach_disk);

    ImGui::Separator();
    if (ImGui::Button("Save Legs Visuals"))
        saveRequested = true;
}

void DebugUI::renderLocomotionPanel(const CMState& /*cmState*/, const CharacterState& charState,
                                    const CharacterConfig& charConfig,
                                    const CharacterReconstructionConfig& /*reconstructionConfig*/,
                                    const Terrain& terrain,
                                    WalkConfig& walkConfig, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Locomotion", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Text("state      = %s", locomotionStateLabel(charState.locomotion_state));
    ImGui::Text("on_floor   = %s", charState.debug_on_floor ? "true" : "false");
    renderWalkParams(walkConfig);

    ImGui::Separator();
    if (ImGui::Button("Save Walk Config"))
        saveRequested = true;
}

void DebugUI::renderBalancePanel(const CMState& /*cmState*/, const CharacterState& charState,
                                  const CharacterConfig& charConfig, const StandingConfig& /*standConfig*/,
                                  CMConfig& cmConfig, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Balance", ImGuiTreeNodeFlags_None))
        return;

    if (!charState.feet_initialized) {
        ImGui::TextDisabled("(awaiting bootstrap)");
        return;
    }

    const double L = charConfig.body_height_m / 5.0;
    renderBalanceReachMetrics(charState, L);
    ImGui::Separator();
    ImGui::Checkbox("Show XCoM line", &cmConfig.show_xcom_line);
    ImGui::Checkbox("Show support line", &cmConfig.show_support_line);

    ImGui::Separator();
    if (ImGui::Button("Save Balance Visuals"))
        saveRequested = true;
}

// ─── Jump ────────────────────────────────────────────────────────────────────

void DebugUI::renderJumpPanel(JumpConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Jump", ImGuiTreeNodeFlags_None))
        return;

    ImGui::TextDisabled("Preload (crouch before takeoff)");
    auto sliderDur = [&](const char* label, double& val) {
        float v = static_cast<float>(val);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat(label, &v, 0.02f, 0.30f, "%.3f s"))
            { val = static_cast<double>(v); saveRequested = true; }
    };
    auto sliderDepth = [&](const char* label, double& val) {
        float v = static_cast<float>(val);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat(label, &v, 0.05f, 0.60f, "%.2f ×L"))
            { val = static_cast<double>(v); saveRequested = true; }
    };
    sliderDur  ("Dur run (s)",         config.preload_dur_run);
    sliderDur  ("Dur walk (s)",        config.preload_dur_walk);
    sliderDur  ("Dur stand (s)",       config.preload_dur_stand);
    sliderDepth("Depth run (×L)",      config.preload_depth_run);
    sliderDepth("Depth walk (×L)",     config.preload_depth_walk);
    sliderDepth("Depth stand (×L)",    config.preload_depth_stand);

    ImGui::Separator();
    ImGui::TextDisabled("Flight");
    sliderDepth("Tuck height (×L)",    config.tuck_height_ratio);

    ImGui::Separator();
    ImGui::TextDisabled("Landing recovery");
    sliderDur  ("Dur jump (s)",        config.landing_dur_jump);
    sliderDur  ("Dur walk land (s)",   config.landing_dur_walk);
    {
        float v = static_cast<float>(config.landing_boost_base_jump);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("Boost base jump", &v, 0.0f, 2.0f, "%.2f"))
            { config.landing_boost_base_jump = static_cast<double>(v); saveRequested = true; }
    }
    {
        float v = static_cast<float>(config.landing_boost_scale_jump);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("Boost scale jump", &v, 0.0f, 3.0f, "%.2f"))
            { config.landing_boost_scale_jump = static_cast<double>(v); saveRequested = true; }
    }
    {
        float v = static_cast<float>(config.landing_boost_base_walk);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("Boost base walk", &v, 0.0f, 2.0f, "%.2f"))
            { config.landing_boost_base_walk = static_cast<double>(v); saveRequested = true; }
    }
    {
        float v = static_cast<float>(config.landing_boost_scale_walk);
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::SliderFloat("Boost scale walk", &v, 0.0f, 3.0f, "%.2f"))
            { config.landing_boost_scale_walk = static_cast<double>(v); saveRequested = true; }
    }

    ImGui::Separator();
    if (ImGui::Button("Save Jump Config"))
        saveRequested = true;
}

// ─── Physics ─────────────────────────────────────────────────────────────────

void DebugUI::renderPhysicsPanel(PhysicsConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_None))
        return;

    renderPhysicsGravityControls(config);

    // Floor friction
    float kf = static_cast<float>(config.floor_friction);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Floor friction (s⁻¹)", &kf, 0.f, 20.f, "%.2f"))
        config.floor_friction = static_cast<double>(kf);

    ImGui::Separator();

    // Vertical tracking (tanh nonlinear)
    renderPhysicsVerticalTrackingControls(config);

    ImGui::Separator();

    // Locomotion
    renderPhysicsLocomotionControls(config);

    ImGui::TextDisabled("Q / D = left / right    SPACE = jump");

    ImGui::Separator();
    if (ImGui::Button("Save Physics Config"))
        saveRequested = true;
}

// ─── Terrain ─────────────────────────────────────────────────────────────────

void DebugUI::renderTerrainPanel(TerrainConfig& config, bool& saveRequested, bool& regenerateRequested)
{
    if (!ImGui::CollapsingHeader("Terrain", ImGuiTreeNodeFlags_None))
        return;

    if (ImGui::Checkbox("Enable terrain", &config.enabled))
        regenerateRequested = true;

    ImGui::BeginDisabled(!config.enabled);
    renderTerrainGenerationControls(config);
    renderTerrainAngleControls(config);
    renderTerrainHeightControls(config);

    ImGui::EndDisabled();

    ImGui::Separator();
    if (ImGui::Button("Regenerate"))
        regenerateRequested = true;
    ImGui::SameLine();
    if (ImGui::Button("Save Terrain Config"))
        saveRequested = true;
}

// ─── Terrain Sampling ────────────────────────────────────────────────────────

void DebugUI::renderTerrainSamplingPanel(TerrainSamplingConfig& config, CMConfig& cmConfig, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Terrain Sampling", ImGuiTreeNodeFlags_None))
        return;

    renderTerrainSamplingControls(config);

    ImGui::Separator();
    ImGui::Checkbox("Show ground reference", &cmConfig.show_ground_reference);

    ImGui::Separator();
    if (ImGui::Button("Save Terrain Sampling Config"))
        saveRequested = true;
}

// ─── IP Completion Test ───────────────────────────────────────────────────────

void DebugUI::renderIPTestPanel(const CMState&        cmState,
                                const CharacterState& charState,
                                const CharacterConfig& charConfig,
                                const StandingConfig& standConfig,
                                const PhysicsConfig&  physConfig,
                                SimulationLoop&       simLoop,
                                AppRequests&         req)
{
    if (!ImGui::CollapsingHeader("IP Completion Test"))
        return;

    const double L         = charConfig.body_height_m / 5.0;
    const double nominal_y = computeNominalY(L, standConfig.d_pref, charConfig.cm_pelvis_ratio);
    const double omega0    = std::sqrt(physConfig.gravity / nominal_y);
    const double mu        = physConfig.floor_friction;

    ImGui::TextDisabled("Tests: inverted pendulum prediction vs actual physics.");
    ImGui::TextDisabled("Requires feet initialized.");
    ImGui::Separator();

    // Launch button — requires feet to be initialized
    if (!charState.feet_initialized)
        ImGui::BeginDisabled();

    if (ImGui::Button("Launch  [CM = L-foot + 0.3L,  vx = 0]") && charState.feet_initialized) {
        const FootState& stance_foot = charState.foot_left;
        const double stance_x  = stance_foot.pos.x;
        const double facing    = charState.facing;
        const double x0_rel    = 0.3 * L;
        const double target_x  = stance_x + facing * x0_rel;

        const double disc = std::sqrt(mu*mu + 4.0*omega0*omega0);
        const double r1   = (-mu + disc) * 0.5;
        const double r2   = (-mu - disc) * 0.5;
        const double v0   = 0.0;
        const double C1   = (v0 - r2*x0_rel) / (r1 - r2);
        const double C2   = (r1*x0_rel - v0) / (r1 - r2);

        m_ipTest = { true,
                     simLoop.getSimulationTime(),
                     stance_x, facing,
                     x0_rel, v0,
                     omega0, mu, r1, r2, C1, C2 };

        req.ip_test_launch = true;
        req.ip_test_cm_x   = target_x;
        req.ip_test_cm_vx  = 0.0;
    }

    if (!charState.feet_initialized) {
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextColored({1,0.4f,0,1}, "No feet yet");
    }

    if (m_ipTest.active && ImGui::Button("Stop test"))
        m_ipTest.active = false;

    if (!m_ipTest.active) return;

    ImGui::Separator();

    const double t = simLoop.getSimulationTime() - m_ipTest.t_start;
    const double exp1   = std::exp(m_ipTest.r1 * t);
    const double exp2   = std::exp(m_ipTest.r2 * t);
    const double x_pred = m_ipTest.C1 * exp1 + m_ipTest.C2 * exp2;
    const double v_pred = m_ipTest.C1 * m_ipTest.r1 * exp1
                        + m_ipTest.C2 * m_ipTest.r2 * exp2;

    const double x_rel_act = (cmState.position.x - m_ipTest.stance_x) * m_ipTest.facing;
    const double v_act     = cmState.velocity.x * m_ipTest.facing;

    const double dx_pred = x_pred - m_ipTest.x0_rel;
    const double dx_act  = x_rel_act - m_ipTest.x0_rel;

    ImGui::Text("t = %.3f s", t);
    ImGui::Text("omega0 = %.3f rad/s   mu = %.2f s^-1", omega0, mu);
    ImGui::Separator();

    ImGui::TextColored({0.4f,1,0.4f,1}, "Analytical (IP + friction):");
    ImGui::Text("  x_pred = stance %+.4f m   delta = %+.4f m", x_pred, dx_pred);
    ImGui::Text("  v_pred =        %+.4f m/s", v_pred);

    ImGui::TextColored({1,0.8f,0.2f,1}, "Actual:");
    ImGui::Text("  x_act  = stance %+.4f m   delta = %+.4f m", x_rel_act, dx_act);
    ImGui::Text("  v_act  =        %+.4f m/s", v_act);

    const double err_x = x_rel_act - x_pred;
    const double err_v = v_act - v_pred;
    ImGui::Text("  error  x = %+.4f m   v = %+.4f m/s", err_x, err_v);
    ImGui::Separator();

    if (dx_act > 0.001)
        ImGui::TextColored({0.2f,1,0.2f,1}, "PASS  IP force moving CM forward  (delta_x = %+.4f m)", dx_act);
    else if (dx_act > -0.001)
        ImGui::TextColored({1,1,0.2f,1}, "WAIT  CM not yet moving...");
    else
        ImGui::TextColored({1,0.2f,0.2f,1}, "FAIL  CM moving backward!");
}
