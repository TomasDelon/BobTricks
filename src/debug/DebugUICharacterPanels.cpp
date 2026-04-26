#include "debug/DebugUI.h"

#include "core/physics/Geometry.h"

#include <cmath>

#include "imgui.h"

namespace {

static bool sliderDouble(const char* label, double& val, float lo, float hi, const char* fmt = "%.2f")
{
    float v = static_cast<float>(val);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat(label, &v, lo, hi, fmt)) {
        val = static_cast<double>(v);
        return true;
    }
    return false;
}

const char* locomotionStateLabel(LocomotionState state)
{
    switch (state) {
        case LocomotionState::Standing: return "Debout";
        case LocomotionState::Walking:  return "Marche";
        case LocomotionState::Running:  return "Course";
        case LocomotionState::Airborne: return "Aerien";
    }
    return "?";
}

void renderLocomotionPoseDebug(const CharacterState&  charState,
                               const CharacterConfig& charConfig,
                               const Terrain&         terrain)
{
    ImGui::Text("etat         = %s", locomotionStateLabel(charState.locomotion_state));
    ImGui::Text("orientation  = %+.2f", charState.facing);
    ImGui::Text("au_sol       = %s", charState.debug_on_floor ? "vrai" : "faux");
    ImGui::Text("cm_target_y  = %+.3f m", charState.debug_cm_target_y);

    const double L        = charConfig.body_height_m / 5.0;
    const double ground_y = terrain.height_at(charState.pelvis.x);
    const double pelvis_h = charState.pelvis.y - ground_y;
    ImGui::Text("hauteur_bassin = %.3f m  (%.2f L)", pelvis_h, pelvis_h / L);
    ImGui::Text("theta          = %+.2f deg  pente_lp = %+.3f",
                charState.theta * 180.0 / 3.14159265358979323846,
                charState.filtered_slope);
}

void renderFootDebug(const char* label, const FootState& foot)
{
    ImGui::Separator();
    ImGui::TextDisabled("Pied %s", label);
    ImGui::Text("pos       = (%+.4f, %+.4f)", foot.pos.x, foot.pos.y);
    if (foot.on_ground)
        ImGui::TextColored({0.2f, 0.9f, 0.2f, 1.f}, "au_sol    = vrai");
    else
        ImGui::TextColored({0.9f, 0.5f, 0.1f, 1.f}, "au_sol    = faux");
    ImGui::Text("normale   = (%.3f, %.3f)", foot.ground_normal.x, foot.ground_normal.y);
}

void renderWalkParams(WalkConfig& walkConfig)
{
    ImGui::Separator();
    ImGui::TextDisabled("Parametres de marche");
    sliderDouble("xcom_scale",              walkConfig.xcom_scale,          0.0f,  1.0f);
    ImGui::SameLine(); ImGui::TextDisabled("alpha*v/w0 dans xi");
    sliderDouble("d_rear_max (xL)",         walkConfig.d_rear_max,          0.5f,  3.0f);
    ImGui::SameLine(); ImGui::TextDisabled("seuil de rattrapage du pied arriere");
    sliderDouble("max_step_L (xL)",         walkConfig.max_step_L,          0.5f,  4.0f);
    ImGui::SameLine(); ImGui::TextDisabled("longueur max depuis le pied d'appui");
    sliderDouble("stability_margin (xL)",   walkConfig.stability_margin,    0.0f,  1.5f);
    sliderDouble("step_speed (pas/s)",      walkConfig.step_speed,          0.5f, 15.0f, "%.1f");
    sliderDouble("cm_height_offset (m)",    walkConfig.cm_height_offset,   -0.3f,  0.3f, "%.3f");
    sliderDouble("double_support_time (s)", walkConfig.double_support_time, 0.0f,  0.15f, "%.3f");
    ImGui::SameLine(); ImGui::TextDisabled("temps minimal en double appui apres heel-strike");
    ImGui::Separator();
    ImGui::TextDisabled("Oscillation IP");
    sliderDouble("leg_flex_coeff (xL)",     walkConfig.leg_flex_coeff,      0.0f,  0.3f, "%.3f");
    ImGui::SameLine(); ImGui::TextDisabled("flexion du genou a la mid-stance");
    sliderDouble("bob_scale",               walkConfig.bob_scale,           0.0f, 10.0f);
    ImGui::SameLine(); ImGui::TextDisabled("multiplicateur d'ecart a l'arc");
    sliderDouble("bob_amp (xL)",            walkConfig.bob_amp,             0.0f,  0.5f, "%.3f");
    ImGui::SameLine(); ImGui::TextDisabled("borne max de l'abaissement");
    ImGui::Separator();
    ImGui::TextDisabled("Levee du pied (h_clear)");
    sliderDouble("h_clear_slope (xL/slope)", walkConfig.h_clear_slope_factor, 0.0f, 2.0f);
    ImGui::SameLine(); ImGui::TextDisabled("levee supplementaire par unite de pente montante");
    sliderDouble("h_clear_speed (xL)",      walkConfig.h_clear_speed_factor, 0.0f, 0.5f, "%.3f");
    ImGui::SameLine(); ImGui::TextDisabled("levee supplementaire a walk_max_speed");
    sliderDouble("h_clear_min (xL)",        walkConfig.h_clear_min_ratio,   0.0f,  0.2f, "%.3f");
    ImGui::SameLine(); ImGui::TextDisabled("levee minimale du pied");
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

    ImGui::Checkbox("Afficher le disque de portee du bassin (2L)", &config.show_pelvis_reach_disk);
}

void renderReconstructionFacingControls(CharacterReconstructionConfig& config)
{
    ImGui::TextDisabled("Facing");
    sliderDouble("Facing deadzone (m/s)", config.facing_eps, 0.01f, 0.5f);
}

void renderReconstructionLocomotionControls(CharacterReconstructionConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Locomotion");
    sliderDouble("Seuil de marche (m/s)", config.walk_eps, 0.01f, 1.0f);
}

void renderReconstructionLeanControls(CharacterReconstructionConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Pelvis lean");
    sliderDouble("Theta max (deg)",     config.theta_max_deg,     0.f, 30.f, "%.1f");
    sliderDouble("V ref (m/s)",         config.v_ref,             0.1f, 5.f);
    sliderDouble("Hunch min (deg)",     config.hunch_min_deg,     0.f, 20.f, "%.1f");
    sliderDouble("Hunch max (deg)",     config.hunch_max_deg,     0.f, 20.f, "%.1f");
    sliderDouble("Hunch current (deg)", config.hunch_current_deg, 0.f, 20.f, "%.1f");
}

void renderBalanceReachMetrics(const CharacterState& charState, double L)
{
    const FootState& lf = charState.foot_left;
    const FootState& rf = charState.foot_right;
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
    ImGui::Checkbox("Trace", &config.show_trail);
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
    sliderDouble("upper_arm_L (xL)",                  config.upper_arm_L,                  0.5f,  1.5f);
    sliderDouble("fore_arm_L (xL)",                   config.fore_arm_L,                   0.5f,  1.5f);
    sliderDouble("walk_hand_reach_reduction_L (xL)",  config.walk_hand_reach_reduction_L,  0.0f,  1.25f);
    sliderDouble("walk_hand_phase_speed_scale",       config.walk_hand_phase_speed_scale,  0.1f,  2.0f);
    sliderDouble("walk_hand_speed_arc_gain",          config.walk_hand_speed_arc_gain,     0.0f,  1.0f);
    sliderDouble("walk_hand_phase_response",          config.walk_hand_phase_response,     1.0f, 30.0f, "%.1f");
    sliderDouble("walk_hand_phase_friction",          config.walk_hand_phase_friction,     0.1f, 12.0f, "%.1f");
    ImGui::Separator();
    ImGui::TextDisabled("Blend vers la course");
    sliderDouble("run_blend_tau (s)",                 config.run_blend_tau,                0.02f, 0.60f);
    sliderDouble("run_hand_reach_reduction_L (xL)",   config.run_hand_reach_reduction_L,   0.0f,  1.25f);
    sliderDouble("run_hand_phase_speed_scale",        config.run_hand_phase_speed_scale,   0.1f,  2.0f);
    sliderDouble("run_hand_speed_arc_gain",           config.run_hand_speed_arc_gain,      0.0f,  1.0f);
    sliderDouble("run_hand_phase_response",           config.run_hand_phase_response,      1.0f, 30.0f, "%.1f");
    sliderDouble("run_hand_phase_friction",           config.run_hand_phase_friction,      0.1f, 12.0f, "%.1f");
}

void renderArmArcControls(ArmConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Front hand arc");
    sliderDouble("front_start_deg",     config.walk_front_hand_start_deg, -180.f, 180.f, "%.1f");
    sliderDouble("front_end_deg",       config.walk_front_hand_end_deg,   -180.f, 180.f, "%.1f");
    ImGui::Separator();
    ImGui::TextDisabled("Back hand arc");
    sliderDouble("back_start_deg",      config.walk_back_hand_start_deg,  -180.f, 180.f, "%.1f");
    sliderDouble("back_end_deg",        config.walk_back_hand_end_deg,    -180.f, 180.f, "%.1f");
    ImGui::Separator();
    ImGui::TextDisabled("Arc avant de la main en course");
    sliderDouble("run_front_start_deg", config.run_front_hand_start_deg,  -180.f, 180.f, "%.1f");
    sliderDouble("run_front_end_deg",   config.run_front_hand_end_deg,    -180.f, 180.f, "%.1f");
    ImGui::Separator();
    ImGui::TextDisabled("Arc arriere de la main en course");
    sliderDouble("run_back_start_deg",  config.run_back_hand_start_deg,   -180.f, 180.f, "%.1f");
    sliderDouble("run_back_end_deg",    config.run_back_hand_end_deg,     -180.f, 180.f, "%.1f");
}

void renderArmOverlayControls(ArmConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Overlay");
    ImGui::Checkbox("Afficher les cercles de portee", &config.show_debug_reach_circles);
    ImGui::Checkbox("Afficher les points de swing", &config.show_debug_swing_points);
    ImGui::Checkbox("Afficher les arcs de swing", &config.show_debug_swing_arcs);
}

} // namespace

void DebugUI::renderCharacterPanel(CharacterConfig& config, const StandingConfig& standConfig, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Caracteristiques du personnage", ImGuiTreeNodeFlags_None))
        return;

    renderCharacterBodyControls(config);

    const double L = config.body_height_m / 5.0;
    ImGui::Text("Longueur de membre : H/5 = %.3f m", L);

    ImGui::Separator();

    renderCharacterPelvisControls(config);

    const double cm_height = computeNominalY(L, standConfig.d_pref, config.cm_pelvis_ratio);
    ImGui::Text("Hauteur nominale du CM : %.3f m", cm_height);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config personnage"))
        saveRequested = true;
}

void DebugUI::renderArmsPanel(ArmConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Bras", ImGuiTreeNodeFlags_None))
        return;

    renderArmReachControls(config);
    renderArmArcControls(config);
    renderArmOverlayControls(config);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config bras"))
        saveRequested = true;
}

void DebugUI::renderHeadPanel(const CharacterState& charState, HeadConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Tete", ImGuiTreeNodeFlags_None))
        return;

    ImGui::TextDisabled("Geometrie");
    ImGui::Text("centre_tete = (%+.3f, %+.3f)", charState.head_center.x, charState.head_center.y);
    ImGui::Text("rayon_tete  = %.3f m", charState.head_radius);
    ImGui::Text("incl_tete   = %+.2f deg", charState.head_tilt * 180.0 / 3.14159265358979323846);

    float offset = static_cast<float>(config.center_offset_L);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("center_offset_L", &offset, 0.1f, 1.0f, "%.2f"))
        config.center_offset_L = static_cast<double>(offset);

    float radius = static_cast<float>(config.radius_L);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("radius_L", &radius, 0.1f, 1.0f, "%.2f"))
        config.radius_L = static_cast<double>(radius);

    ImGui::Separator();
    ImGui::TextDisabled("Regard / intention");
    float tilt = static_cast<float>(config.max_tilt_deg);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("max_tilt_deg", &tilt, 0.f, 60.f, "%.1f"))
        config.max_tilt_deg = static_cast<double>(tilt);

    float tau = static_cast<float>(config.tau_tilt);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("tau_tilt", &tau, 0.01f, 1.0f, "%.2f"))
        config.tau_tilt = static_cast<double>(tau);

    ImGui::Checkbox("Afficher le repere de l'oeil", &config.show_eye_marker);
    ImGui::Checkbox("Afficher le rayon du regard", &config.show_gaze_ray);
    ImGui::Checkbox("Afficher la cible du regard", &config.show_gaze_target);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config tete"))
        saveRequested = true;
}

void DebugUI::renderReconstructionPanel(CharacterReconstructionConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Reconstruction du personnage", ImGuiTreeNodeFlags_None))
        return;

    renderReconstructionFacingControls(config);
    renderReconstructionLocomotionControls(config);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config reconstruction"))
        saveRequested = true;
}

void DebugUI::renderCMKinematicsPanel(const CMState& state, CMConfig& config, bool& saveRequested, bool& clearTrail)
{
    if (!ImGui::CollapsingHeader("Centre de masse", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Text("Position      x: %+.3f m    y: %.3f m",
                state.position.x, state.position.y);
    ImGui::Text("Vitesse      vx: %+.3f m/s  vy: %+.3f m/s  v: %.3f m/s",
                state.velocity.x, state.velocity.y, state.velocity.length());
    ImGui::Text("Acceleration ax: %+.3f m/s%c ay: %+.3f m/s%c a: %.3f m/s%c",
                state.acceleration.x, static_cast<char>(178),
                state.acceleration.y, static_cast<char>(178),
                state.acceleration.length(), static_cast<char>(178));

    ImGui::Separator();
    ImGui::Checkbox("Ligne de projection",     &config.show_projection_line);
    ImGui::Checkbox("Point de projection",     &config.show_projection_dot);
    ImGui::Checkbox("Repere de hauteur cible", &config.show_target_height_tick);

    ImGui::Separator();
    renderVisualizationVectorControls(config);

    ImGui::Separator();
    renderVisualizationTrailControls(config, clearTrail);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config CM"))
        saveRequested = true;
}

void DebugUI::renderTorsoPanel(const CharacterState& charState,
                               const CharacterConfig& charConfig,
                               CharacterReconstructionConfig& reconstructionConfig,
                               const Terrain& terrain,
                               bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Torse", ImGuiTreeNodeFlags_None))
        return;

    renderLocomotionPoseDebug(charState, charConfig, terrain);

    ImGui::Separator();
    renderReconstructionLeanControls(reconstructionConfig);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config torse"))
        saveRequested = true;
}

void DebugUI::renderLegsPanel(const CharacterState& charState,
                              CharacterConfig& charConfig,
                              CMConfig& /*cmConfig*/,
                              bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Jambes", ImGuiTreeNodeFlags_None))
        return;

    if (!charState.feet_initialized) {
        ImGui::TextDisabled("(en attente du bootstrap)");
    } else {
        renderFootDebug("gauche", charState.foot_left);
        renderFootDebug("droit", charState.foot_right);
    }

    ImGui::Separator();
    ImGui::Checkbox("Afficher le disque de portee du bassin (2L)", &charConfig.show_pelvis_reach_disk);

    ImGui::Separator();
    if (ImGui::Button("Sauver les visuels des jambes"))
        saveRequested = true;
}

void DebugUI::renderLocomotionPanel(const CMState& /*cmState*/, const CharacterState& charState,
                                    const CharacterConfig&,
                                    const CharacterReconstructionConfig& /*reconstructionConfig*/,
                                    const Terrain&,
                                    WalkConfig& walkConfig, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Locomotion", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Text("etat      = %s", locomotionStateLabel(charState.locomotion_state));
    ImGui::Text("au_sol    = %s", charState.debug_on_floor ? "vrai" : "faux");
    renderWalkParams(walkConfig);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config marche"))
        saveRequested = true;
}

void DebugUI::renderBalancePanel(const CMState& /*cmState*/, const CharacterState& charState,
                                 const CharacterConfig& charConfig, const StandingConfig& /*standConfig*/,
                                 CMConfig& cmConfig, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Equilibre", ImGuiTreeNodeFlags_None))
        return;

    if (!charState.feet_initialized) {
        ImGui::TextDisabled("(en attente du bootstrap)");
        return;
    }

    const double L = charConfig.body_height_m / 5.0;
    renderBalanceReachMetrics(charState, L);
    ImGui::Separator();
    ImGui::Checkbox("Afficher la ligne du CM extrapole", &cmConfig.show_xcom_line);
    ImGui::Checkbox("Afficher la ligne de support", &cmConfig.show_support_line);

    ImGui::Separator();
    if (ImGui::Button("Sauver les visuels de balance"))
        saveRequested = true;
}
