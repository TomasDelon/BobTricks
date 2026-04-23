#include "config/ConfigIO.h"

#include <cstdio>
#include <fstream>
#include <string>

namespace
{

std::string trim(const std::string& s)
{
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return {};
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

} // namespace

bool ConfigIO::load(const std::string& path, AppConfig& config)
{
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string section;
    std::string line;
    int line_no = 0;

    while (std::getline(file, line)) {
        ++line_no;
        line = trim(line);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }

        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        const std::string key   = trim(line.substr(0, eq));
        const std::string value = trim(line.substr(eq + 1));
        bool handled = false;

        if (section == "SimLoop") {
            if      (key == "max_fps")    { config.sim_loop.max_fps    = std::stoi(value); handled = true; }
            else if (key == "fixed_dt_s") { config.sim_loop.fixed_dt_s = std::stod(value); handled = true; }
            else if (key == "time_scale") { config.sim_loop.time_scale = std::stod(value); handled = true; }
        }
        else if (section == "Camera") {
            if      (key == "zoom")     { config.camera.zoom     = std::stod(value); handled = true; }
            else if (key == "follow_x") { config.camera.follow_x = (value == "1" || value == "true"); handled = true; }
            else if (key == "follow_y") { config.camera.follow_y = (value == "1" || value == "true"); handled = true; }
            else if (key == "smooth_x") { config.camera.smooth_x = std::stod(value); handled = true; }
            else if (key == "smooth_y") { config.camera.smooth_y = std::stod(value); handled = true; }
            else if (key == "deadzone_x") { config.camera.deadzone_x = std::stod(value); handled = true; }
            else if (key == "deadzone_y") { config.camera.deadzone_y = std::stod(value); handled = true; }
        }
        else if (section == "Character") {
            if      (key == "body_height_m")   { config.character.body_height_m   = std::stod(value); handled = true; }
            else if (key == "cm_pelvis_ratio") { config.character.cm_pelvis_ratio = std::stod(value); handled = true; }
            else if (key == "show_pelvis_reach_disk") { config.character.show_pelvis_reach_disk = (value == "1" || value == "true"); handled = true; }
        }
        else if (section == "Reconstruction") {
            if      (key == "facing_eps")        { config.reconstruction.facing_eps        = std::stod(value); handled = true; }
            else if (key == "walk_eps")          { config.reconstruction.walk_eps          = std::stod(value); handled = true; }
            else if (key == "theta_max_deg")     { config.reconstruction.theta_max_deg     = std::stod(value); handled = true; }
            else if (key == "v_ref")             { config.reconstruction.v_ref             = std::stod(value); handled = true; }
            else if (key == "tau_lean")          { config.reconstruction.tau_lean          = std::stod(value); handled = true; }
            else if (key == "tau_slope")         { config.reconstruction.tau_slope         = std::stod(value); handled = true; }
            else if (key == "slope_lean_factor") { config.reconstruction.slope_lean_factor = std::stod(value); handled = true; }
            else if (key == "hunch_min_deg")     { config.reconstruction.hunch_min_deg     = std::stod(value); handled = true; }
            else if (key == "hunch_max_deg")     { config.reconstruction.hunch_max_deg     = std::stod(value); handled = true; }
            else if (key == "hunch_current_deg") { config.reconstruction.hunch_current_deg = std::stod(value); handled = true; }
        }
        else if (section == "Head") {
            if      (key == "center_offset_L")  { config.head.center_offset_L  = std::stod(value); handled = true; }
            else if (key == "radius_L")         { config.head.radius_L         = std::stod(value); handled = true; }
            else if (key == "eye_height_ratio") { config.head.eye_height_ratio = std::stod(value); handled = true; }
            else if (key == "eye_spacing")      { config.head.eye_spacing      = std::stod(value); handled = true; }
            else if (key == "max_tilt_deg")     { config.head.max_tilt_deg     = std::stod(value); handled = true; }
            else if (key == "tau_tilt")         { config.head.tau_tilt         = std::stod(value); handled = true; }
            else if (key == "show_eye_marker")  { config.head.show_eye_marker  = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_gaze_ray")    { config.head.show_gaze_ray    = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_gaze_target") { config.head.show_gaze_target = (value == "1" || value == "true"); handled = true; }
        }
        else if (section == "Arms") {
            if      (key == "upper_arm_L")                 { config.arms.upper_arm_L                 = std::stod(value); handled = true; }
            else if (key == "fore_arm_L")                  { config.arms.fore_arm_L                  = std::stod(value); handled = true; }
            else if (key == "walk_hand_reach_reduction_L") { config.arms.walk_hand_reach_reduction_L = std::stod(value); handled = true; }
            else if (key == "walk_front_hand_start_deg")   { config.arms.walk_front_hand_start_deg   = std::stod(value); handled = true; }
            else if (key == "walk_front_hand_end_deg")     { config.arms.walk_front_hand_end_deg     = std::stod(value); handled = true; }
            else if (key == "walk_back_hand_start_deg")    { config.arms.walk_back_hand_start_deg    = std::stod(value); handled = true; }
            else if (key == "walk_back_hand_end_deg")      { config.arms.walk_back_hand_end_deg      = std::stod(value); handled = true; }
            else if (key == "walk_hand_phase_speed_scale") { config.arms.walk_hand_phase_speed_scale = std::stod(value); handled = true; }
            else if (key == "walk_hand_speed_arc_gain")    { config.arms.walk_hand_speed_arc_gain    = std::stod(value); handled = true; }
            else if (key == "walk_hand_phase_response")    { config.arms.walk_hand_phase_response    = std::stod(value); handled = true; }
            else if (key == "walk_hand_phase_friction")    { config.arms.walk_hand_phase_friction    = std::stod(value); handled = true; }
            else if (key == "run_hand_reach_reduction_L")  { config.arms.run_hand_reach_reduction_L  = std::stod(value); handled = true; }
            else if (key == "run_front_hand_start_deg")    { config.arms.run_front_hand_start_deg    = std::stod(value); handled = true; }
            else if (key == "run_front_hand_end_deg")      { config.arms.run_front_hand_end_deg      = std::stod(value); handled = true; }
            else if (key == "run_back_hand_start_deg")     { config.arms.run_back_hand_start_deg     = std::stod(value); handled = true; }
            else if (key == "run_back_hand_end_deg")       { config.arms.run_back_hand_end_deg       = std::stod(value); handled = true; }
            else if (key == "run_hand_phase_speed_scale")  { config.arms.run_hand_phase_speed_scale  = std::stod(value); handled = true; }
            else if (key == "run_hand_speed_arc_gain")     { config.arms.run_hand_speed_arc_gain     = std::stod(value); handled = true; }
            else if (key == "run_hand_phase_response")     { config.arms.run_hand_phase_response     = std::stod(value); handled = true; }
            else if (key == "run_hand_phase_friction")     { config.arms.run_hand_phase_friction     = std::stod(value); handled = true; }
            else if (key == "run_blend_tau")               { config.arms.run_blend_tau               = std::stod(value); handled = true; }
            else if (key == "show_debug_reach_circles")    { config.arms.show_debug_reach_circles    = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_debug_swing_points")     { config.arms.show_debug_swing_points     = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_debug_swing_arcs")       { config.arms.show_debug_swing_arcs       = (value == "1" || value == "true"); handled = true; }
        }
        else if (section == "SplineRender") {
            if      (key == "enabled")              { config.spline_render.enabled              = (value == "1" || value == "true"); handled = true; }
            else if (key == "draw_under_legacy")    { config.spline_render.draw_under_legacy    = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_head")            { config.spline_render.show_head            = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_torso")           { config.spline_render.show_torso           = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_arms")            { config.spline_render.show_arms            = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_legs")            { config.spline_render.show_legs            = (value == "1" || value == "true"); handled = true; }
            else if (key == "stroke_width_px")      { config.spline_render.stroke_width_px      = std::stof(value); handled = true; }
            else if (key == "samples_per_curve")    { config.spline_render.samples_per_curve    = std::stoi(value); handled = true; }
            else if (key == "show_test_curve")      { config.spline_render.show_test_curve      = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_control_polygon") { config.spline_render.show_control_polygon = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_sample_points")   { config.spline_render.show_sample_points   = (value == "1" || value == "true"); handled = true; }
        }
        else if (section == "Presentation") {
            if      (key == "show_spline_renderer")         { config.presentation.show_spline_renderer         = (value == "1" || value == "true"); handled = true; }
            else if (key == "force_spline_renderer")        { config.presentation.show_spline_renderer         = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_legacy_skeleton")         { config.presentation.show_legacy_skeleton         = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_character_debug_markers") { config.presentation.show_character_debug_markers = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_pelvis_reach_disk")       { config.presentation.show_pelvis_reach_disk       = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_trail_overlay")           { config.presentation.show_trail_overlay           = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_ground_reference")        { config.presentation.show_ground_reference        = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_cm_projection")           { config.presentation.show_cm_projection           = (value == "1" || value == "true"); handled = true; }
            else if (key == "velocity_components")          { config.presentation.velocity_components          = std::stoi(value); handled = true; }
            else if (key == "accel_components")             { config.presentation.accel_components             = std::stoi(value); handled = true; }
            else if (key == "debug_thickness_scale")        { config.presentation.debug_thickness_scale        = std::stof(value); handled = true; }
            else if (key == "show_cm_vectors")              { const bool show = (value == "1" || value == "true"); config.presentation.velocity_components = show ? 3 : 0; config.presentation.accel_components = show ? 3 : 0; handled = true; }
            else if (key == "show_xcom_overlay")            { config.presentation.show_xcom_overlay            = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_head_overlay")            { config.presentation.show_head_overlay            = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_arm_overlay")             { config.presentation.show_arm_overlay             = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_spline_debug_overlay")    { config.presentation.show_spline_debug_overlay    = (value == "1" || value == "true"); handled = true; }
            // Backward-compatible keys from the former presentation preset.
            else if (key == "hide_head_debug")       { config.presentation.show_head_overlay         = !(value == "1" || value == "true"); handled = true; }
            else if (key == "hide_arm_debug")        { config.presentation.show_arm_overlay          = !(value == "1" || value == "true"); handled = true; }
            else if (key == "hide_cm_debug")         { const bool show = !(value == "1" || value == "true"); config.presentation.show_ground_reference = show; config.presentation.show_cm_projection = show; config.presentation.velocity_components = show ? 3 : 0; config.presentation.accel_components = show ? 3 : 0; config.presentation.show_trail_overlay = show; handled = true; }
            else if (key == "hide_balance_debug")    { config.presentation.show_xcom_overlay         = !(value == "1" || value == "true"); handled = true; }
            else if (key == "hide_spline_debug")     { config.presentation.show_spline_debug_overlay = !(value == "1" || value == "true"); handled = true; }
        }
        else if (section == "CM") {
            if      (key == "show_ground_reference")   { config.cm.show_ground_reference   = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_projection_line")    { config.cm.show_projection_line    = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_projection_dot")     { config.cm.show_projection_dot     = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_target_height_tick") { config.cm.show_target_height_tick = (value == "1" || value == "true"); handled = true; }
            else if (key == "velocity_components")     { config.cm.velocity_components     = std::stoi(value); handled = true; }
            else if (key == "accel_components")        { config.cm.accel_components        = std::stoi(value); handled = true; }
            else if (key == "show_trail")              { config.cm.show_trail              = (value == "1" || value == "true"); handled = true; }
            else if (key == "trail_duration")          { config.cm.trail_duration          = std::stod(value); handled = true; }
            else if (key == "show_xcom_line")          { config.cm.show_xcom_line          = (value == "1" || value == "true"); handled = true; }
            else if (key == "show_support_line")       { config.cm.show_support_line       = (value == "1" || value == "true"); handled = true; }
        }
        else if (section == "Physics") {
            if      (key == "gravity_enabled") { config.physics.gravity_enabled  = (value == "1" || value == "true"); handled = true; }
            else if (key == "gravity")         { config.physics.gravity          = std::stod(value); handled = true; }
            else if (key == "accel")           { config.physics.accel            = std::stod(value); handled = true; }
            else if (key == "walk_max_speed")  { config.physics.walk_max_speed   = std::stod(value); handled = true; }
            else if (key == "floor_friction")  { config.physics.floor_friction   = std::stod(value); handled = true; }
            else if (key == "hold_speed")      { config.physics.hold_speed       = std::stod(value); handled = true; }
            else if (key == "stop_speed")      { config.physics.stop_speed       = std::stod(value); handled = true; }
            else if (key == "spring_enabled")  { config.physics.spring_enabled   = (value == "1" || value == "true"); handled = true; }
            else if (key == "vy_max")          { config.physics.vy_max           = std::stod(value); handled = true; }
            else if (key == "d_soft")          { config.physics.d_soft           = std::stod(value); handled = true; }
            else if (key == "vy_tau")          { config.physics.vy_tau           = std::stod(value); handled = true; }
            else if (key == "jump_impulse")    { config.physics.jump_impulse     = std::stod(value); handled = true; }
        }
        else if (section == "TerrainSampling") {
            if      (key == "w_back")    { config.terrain_sampling.w_back    = std::stod(value); handled = true; }
            else if (key == "w_fwd")     { config.terrain_sampling.w_fwd     = std::stod(value); handled = true; }
            else if (key == "t_look")    { config.terrain_sampling.t_look    = std::stod(value); handled = true; }
            else if (key == "tau_slide") { config.terrain_sampling.tau_slide = std::stod(value); handled = true; }
        }
        else if (section == "Terrain") {
            if      (key == "enabled")     { config.terrain.enabled     = (value == "1" || value == "true"); handled = true; }
            else if (key == "seed")        { config.terrain.seed        = std::stoi(value); handled = true; }
            else if (key == "seg_min")     { config.terrain.seg_min     = std::stod(value); handled = true; }
            else if (key == "seg_max")     { config.terrain.seg_max     = std::stod(value); handled = true; }
            else if (key == "angle_small") { config.terrain.angle_small = std::stod(value); handled = true; }
            else if (key == "angle_large") { config.terrain.angle_large = std::stod(value); handled = true; }
            else if (key == "large_prob")  { config.terrain.large_prob  = std::stod(value); handled = true; }
            else if (key == "slope_max")   { config.terrain.slope_max   = std::stod(value); handled = true; }
            else if (key == "height_min")  { config.terrain.height_min  = std::stod(value); handled = true; }
            else if (key == "height_max")  { config.terrain.height_max  = std::stod(value); handled = true; }
        }
        else if (section == "Particles") {
            if      (key == "enabled")        { config.particles.enabled        = (value == "1" || value == "true"); handled = true; }
            else if (key == "dust_enabled")   { config.particles.dust_enabled   = (value == "1" || value == "true"); handled = true; }
            else if (key == "impact_enabled") { config.particles.impact_enabled = (value == "1" || value == "true"); handled = true; }
            else if (key == "slide_enabled")  { config.particles.slide_enabled  = (value == "1" || value == "true"); handled = true; }
            else if (key == "landing_enabled"){ config.particles.landing_enabled= (value == "1" || value == "true"); handled = true; }
            else if (key == "dust_burst_count"){ config.particles.dust_burst_count = std::stoi(value); handled = true; }
            else if (key == "dust_radius_px") { config.particles.dust_radius_px = std::stof(value); handled = true; }
            else if (key == "dust_alpha")     { config.particles.dust_alpha     = std::stof(value); handled = true; }
            else if (key == "dust_lifetime_s"){ config.particles.dust_lifetime_s = std::stod(value); handled = true; }
            else if (key == "dust_speed_mps") { config.particles.dust_speed_mps = std::stod(value); handled = true; }
            else if (key == "slide_emit_interval_s") { config.particles.slide_emit_interval_s = std::stod(value); handled = true; }
            else if (key == "landing_burst_scale")   { config.particles.landing_burst_scale   = std::stod(value); handled = true; }
        }
        else if (section == "Audio") {
            if      (key == "footstep_volume") { config.audio.footstep_volume = std::stod(value); handled = true; }
            else if (key == "music_volume")    { config.audio.music_volume    = std::stod(value); handled = true; }
            else if (key == "music_enabled")   { config.audio.music_enabled   = (value == "1" || value == "true"); handled = true; }
            else if (key == "music_track")     { config.audio.music_track     = std::stoi(value); handled = true; }
        }
        else if (section == "Standing") {
            if      (key == "d_pref")   { config.standing.d_pref   = std::stod(value); handled = true; }
            else if (key == "d_min")    { config.standing.d_min    = std::stod(value); handled = true; }
            else if (key == "d_max")    { config.standing.d_max    = std::stod(value); handled = true; }
            else if (key == "eps_v")    { config.standing.eps_v    = std::stod(value); handled = true; }
        }
        else if (section == "Step") {
            if (key == "h_clear_ratio") { config.step.h_clear_ratio = std::stod(value); handled = true; }
        }
        else if (section == "Walk") {
            if      (key == "eps_step")            { config.walk.eps_step            = std::stod(value); handled = true; }
            else if (key == "xcom_scale")          { config.walk.xcom_scale          = std::stod(value); handled = true; }
            else if (key == "d_rear_max")          { config.walk.d_rear_max          = std::stod(value); handled = true; }
            else if (key == "max_step_L")          { config.walk.max_step_L          = std::stod(value); handled = true; }
            else if (key == "max_speed_drop")      { config.walk.max_speed_drop      = std::stod(value); handled = true; }
            else if (key == "max_slope_drop")      { config.walk.max_slope_drop      = std::stod(value); handled = true; }
            else if (key == "downhill_crouch_max") { config.walk.downhill_crouch_max = std::stod(value); handled = true; }
            else if (key == "downhill_crouch_tau") { config.walk.downhill_crouch_tau = std::stod(value); handled = true; }
            else if (key == "downhill_relax_tau")  { config.walk.downhill_relax_tau  = std::stod(value); handled = true; }
            else if (key == "downhill_step_bonus") { config.walk.downhill_step_bonus = std::stod(value); handled = true; }
            else if (key == "step_speed")          { config.walk.step_speed          = std::stod(value); handled = true; }
            else if (key == "stability_margin")    { config.walk.stability_margin    = std::stod(value); handled = true; }
            else if (key == "double_support_time") { config.walk.double_support_time = std::stod(value); handled = true; }
            else if (key == "cm_height_offset")    { config.walk.cm_height_offset    = std::stod(value); handled = true; }
            else if (key == "leg_flex_coeff")      { config.walk.leg_flex_coeff      = std::stod(value); handled = true; }
            else if (key == "bob_scale")           { config.walk.bob_scale           = std::stod(value); handled = true; }
            else if (key == "bob_amp")             { config.walk.bob_amp             = std::stod(value); handled = true; }
            else if (key == "h_clear_slope_factor"){ config.walk.h_clear_slope_factor= std::stod(value); handled = true; }
            else if (key == "h_clear_speed_factor"){ config.walk.h_clear_speed_factor= std::stod(value); handled = true; }
            else if (key == "h_clear_min_ratio")   { config.walk.h_clear_min_ratio   = std::stod(value); handled = true; }
        }
        else if (section == "Run") {
            if      (key == "max_speed")           { config.run.max_speed           = std::stod(value); handled = true; }
            else if (key == "accel_factor")        { config.run.accel_factor        = std::stod(value); handled = true; }
            else if (key == "step_speed")          { config.run.step_speed          = std::stod(value); handled = true; }
            else if (key == "stability_margin")    { config.run.stability_margin    = std::stod(value); handled = true; }
            else if (key == "max_step_L")          { config.run.max_step_L          = std::stod(value); handled = true; }
            else if (key == "d_rear_max")          { config.run.d_rear_max          = std::stod(value); handled = true; }
            else if (key == "xcom_scale")          { config.run.xcom_scale          = std::stod(value); handled = true; }
            else if (key == "stride_len")          { config.run.stride_len          = std::stod(value); handled = true; }
            else if (key == "stride_len_min")      { config.run.stride_len_min      = std::stod(value); handled = true; }
            else if (key == "cadence_spm_min")     { config.run.cadence_spm_min     = std::stod(value); handled = true; }
            else if (key == "cadence_spm_max")     { config.run.cadence_spm_max     = std::stod(value); handled = true; }
            else if (key == "theta_max_deg")       { config.run.theta_max_deg       = std::stod(value); handled = true; }
            else if (key == "leg_flex_coeff")      { config.run.leg_flex_coeff      = std::stod(value); handled = true; }
            else if (key == "bob_scale")           { config.run.bob_scale           = std::stod(value); handled = true; }
            else if (key == "bob_amp")             { config.run.bob_amp             = std::stod(value); handled = true; }
            else if (key == "h_clear_ratio")       { config.run.h_clear_ratio       = std::stod(value); handled = true; }
            else if (key == "h_clear_min_ratio")   { config.run.h_clear_min_ratio   = std::stod(value); handled = true; }
            else if (key == "blend_tau")           { config.run.blend_tau           = std::stod(value); handled = true; }
        }
        else if (section == "Jump") {
            if      (key == "preload_dur_run")          { config.jump.preload_dur_run          = std::stod(value); handled = true; }
            else if (key == "preload_dur_walk")         { config.jump.preload_dur_walk         = std::stod(value); handled = true; }
            else if (key == "preload_dur_stand")        { config.jump.preload_dur_stand        = std::stod(value); handled = true; }
            else if (key == "preload_depth_run")        { config.jump.preload_depth_run        = std::stod(value); handled = true; }
            else if (key == "preload_depth_walk")       { config.jump.preload_depth_walk       = std::stod(value); handled = true; }
            else if (key == "preload_depth_stand")      { config.jump.preload_depth_stand      = std::stod(value); handled = true; }
            else if (key == "tuck_height_ratio")        { config.jump.tuck_height_ratio        = std::stod(value); handled = true; }
            else if (key == "landing_dur_jump")         { config.jump.landing_dur_jump         = std::stod(value); handled = true; }
            else if (key == "landing_dur_walk")         { config.jump.landing_dur_walk         = std::stod(value); handled = true; }
            else if (key == "landing_boost_base_jump")  { config.jump.landing_boost_base_jump  = std::stod(value); handled = true; }
            else if (key == "landing_boost_scale_jump") { config.jump.landing_boost_scale_jump = std::stod(value); handled = true; }
            else if (key == "landing_boost_base_walk")  { config.jump.landing_boost_base_walk  = std::stod(value); handled = true; }
            else if (key == "landing_boost_scale_walk") { config.jump.landing_boost_scale_walk = std::stod(value); handled = true; }
        }
        if (!handled) {
            std::fprintf(stderr,
                         "[ConfigIO] Ignoring unknown key '%s' in section [%s] at line %d\n",
                         key.c_str(), section.c_str(), line_no);
        }
    }

    return true;
}

bool ConfigIO::save(const std::string& path, const AppConfig& config)
{
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "; BobTricks — configuration file\n\n";

    file << "[SimLoop]\n";
    file << "max_fps="    << config.sim_loop.max_fps    << "\n";
    file << "fixed_dt_s=" << config.sim_loop.fixed_dt_s << "\n";
    file << "time_scale=" << config.sim_loop.time_scale << "\n\n";

    file << "[Camera]\n";
    file << "zoom="     << config.camera.zoom                     << "\n";
    file << "follow_x=" << (config.camera.follow_x ? "1" : "0")  << "\n";
    file << "follow_y=" << (config.camera.follow_y ? "1" : "0")  << "\n";
    file << "smooth_x=" << config.camera.smooth_x                 << "\n";
    file << "smooth_y=" << config.camera.smooth_y                 << "\n";
    file << "deadzone_x=" << config.camera.deadzone_x             << "\n";
    file << "deadzone_y=" << config.camera.deadzone_y             << "\n\n";

    file << "[Character]\n";
    file << "body_height_m="   << config.character.body_height_m   << "\n";
    file << "cm_pelvis_ratio=" << config.character.cm_pelvis_ratio << "\n";
    file << "show_pelvis_reach_disk=" << (config.character.show_pelvis_reach_disk ? "1" : "0") << "\n\n";

    file << "[Reconstruction]\n";
    file << "facing_eps="        << config.reconstruction.facing_eps        << "\n";
    file << "walk_eps="          << config.reconstruction.walk_eps          << "\n";
    file << "theta_max_deg="     << config.reconstruction.theta_max_deg     << "\n";
    file << "v_ref="             << config.reconstruction.v_ref             << "\n";
    file << "tau_lean="          << config.reconstruction.tau_lean          << "\n";
    file << "tau_slope="         << config.reconstruction.tau_slope         << "\n";
    file << "slope_lean_factor=" << config.reconstruction.slope_lean_factor << "\n";
    file << "hunch_min_deg="     << config.reconstruction.hunch_min_deg     << "\n";
    file << "hunch_max_deg="     << config.reconstruction.hunch_max_deg     << "\n";
    file << "hunch_current_deg=" << config.reconstruction.hunch_current_deg << "\n\n";

    file << "[Head]\n";
    file << "center_offset_L="  << config.head.center_offset_L  << "\n";
    file << "radius_L="         << config.head.radius_L         << "\n";
    file << "eye_height_ratio=" << config.head.eye_height_ratio << "\n";
    file << "eye_spacing="      << config.head.eye_spacing      << "\n";
    file << "max_tilt_deg="     << config.head.max_tilt_deg     << "\n";
    file << "tau_tilt="         << config.head.tau_tilt         << "\n";
    file << "show_eye_marker="  << (config.head.show_eye_marker ? "1" : "0") << "\n";
    file << "show_gaze_ray="    << (config.head.show_gaze_ray ? "1" : "0") << "\n";
    file << "show_gaze_target=" << (config.head.show_gaze_target ? "1" : "0") << "\n\n";

    file << "[Arms]\n";
    file << "upper_arm_L="                 << config.arms.upper_arm_L                 << "\n";
    file << "fore_arm_L="                  << config.arms.fore_arm_L                  << "\n";
    file << "walk_hand_reach_reduction_L=" << config.arms.walk_hand_reach_reduction_L << "\n";
    file << "walk_front_hand_start_deg="   << config.arms.walk_front_hand_start_deg   << "\n";
    file << "walk_front_hand_end_deg="     << config.arms.walk_front_hand_end_deg     << "\n";
    file << "walk_back_hand_start_deg="    << config.arms.walk_back_hand_start_deg    << "\n";
    file << "walk_back_hand_end_deg="      << config.arms.walk_back_hand_end_deg      << "\n";
    file << "walk_hand_phase_speed_scale=" << config.arms.walk_hand_phase_speed_scale << "\n";
    file << "walk_hand_speed_arc_gain="    << config.arms.walk_hand_speed_arc_gain    << "\n";
    file << "walk_hand_phase_response="    << config.arms.walk_hand_phase_response    << "\n";
    file << "walk_hand_phase_friction="    << config.arms.walk_hand_phase_friction    << "\n";
    file << "run_hand_reach_reduction_L="  << config.arms.run_hand_reach_reduction_L  << "\n";
    file << "run_front_hand_start_deg="    << config.arms.run_front_hand_start_deg    << "\n";
    file << "run_front_hand_end_deg="      << config.arms.run_front_hand_end_deg      << "\n";
    file << "run_back_hand_start_deg="     << config.arms.run_back_hand_start_deg     << "\n";
    file << "run_back_hand_end_deg="       << config.arms.run_back_hand_end_deg       << "\n";
    file << "run_hand_phase_speed_scale="  << config.arms.run_hand_phase_speed_scale  << "\n";
    file << "run_hand_speed_arc_gain="     << config.arms.run_hand_speed_arc_gain     << "\n";
    file << "run_hand_phase_response="     << config.arms.run_hand_phase_response     << "\n";
    file << "run_hand_phase_friction="     << config.arms.run_hand_phase_friction     << "\n";
    file << "run_blend_tau="               << config.arms.run_blend_tau               << "\n";
    file << "show_debug_reach_circles="    << (config.arms.show_debug_reach_circles ? "1" : "0") << "\n";
    file << "show_debug_swing_points="     << (config.arms.show_debug_swing_points ? "1" : "0") << "\n";
    file << "show_debug_swing_arcs="       << (config.arms.show_debug_swing_arcs ? "1" : "0") << "\n\n";

    file << "[SplineRender]\n";
    file << "enabled="              << (config.spline_render.enabled ? "1" : "0") << "\n";
    file << "draw_under_legacy="    << (config.spline_render.draw_under_legacy ? "1" : "0") << "\n";
    file << "show_head="            << (config.spline_render.show_head ? "1" : "0") << "\n";
    file << "show_torso="           << (config.spline_render.show_torso ? "1" : "0") << "\n";
    file << "show_arms="            << (config.spline_render.show_arms ? "1" : "0") << "\n";
    file << "show_legs="            << (config.spline_render.show_legs ? "1" : "0") << "\n";
    file << "stroke_width_px="      << config.spline_render.stroke_width_px << "\n";
    file << "samples_per_curve="    << config.spline_render.samples_per_curve << "\n";
    file << "show_test_curve="      << (config.spline_render.show_test_curve ? "1" : "0") << "\n";
    file << "show_control_polygon=" << (config.spline_render.show_control_polygon ? "1" : "0") << "\n";
    file << "show_sample_points="   << (config.spline_render.show_sample_points ? "1" : "0") << "\n\n";

    file << "[Presentation]\n";
    file << "show_spline_renderer="         << (config.presentation.show_spline_renderer ? "1" : "0") << "\n";
    file << "show_legacy_skeleton="         << (config.presentation.show_legacy_skeleton ? "1" : "0") << "\n";
    file << "show_character_debug_markers=" << (config.presentation.show_character_debug_markers ? "1" : "0") << "\n";
    file << "show_pelvis_reach_disk="       << (config.presentation.show_pelvis_reach_disk ? "1" : "0") << "\n";
    file << "show_trail_overlay="           << (config.presentation.show_trail_overlay ? "1" : "0") << "\n";
    file << "show_ground_reference="        << (config.presentation.show_ground_reference ? "1" : "0") << "\n";
    file << "show_cm_projection="           << (config.presentation.show_cm_projection ? "1" : "0") << "\n";
    file << "velocity_components="          << config.presentation.velocity_components << "\n";
    file << "accel_components="             << config.presentation.accel_components << "\n";
    file << "debug_thickness_scale="        << config.presentation.debug_thickness_scale << "\n";
    file << "show_xcom_overlay="            << (config.presentation.show_xcom_overlay ? "1" : "0") << "\n";
    file << "show_head_overlay="            << (config.presentation.show_head_overlay ? "1" : "0") << "\n";
    file << "show_arm_overlay="             << (config.presentation.show_arm_overlay ? "1" : "0") << "\n";
    file << "show_spline_debug_overlay="    << (config.presentation.show_spline_debug_overlay ? "1" : "0") << "\n\n";

    file << "[CM]\n";
    file << "show_ground_reference="   << (config.cm.show_ground_reference   ? "1" : "0") << "\n";
    file << "show_projection_line="    << (config.cm.show_projection_line    ? "1" : "0") << "\n";
    file << "show_projection_dot="     << (config.cm.show_projection_dot     ? "1" : "0") << "\n";
    file << "show_target_height_tick=" << (config.cm.show_target_height_tick ? "1" : "0") << "\n";
    file << "velocity_components="     << config.cm.velocity_components     << "\n";
    file << "accel_components="        << config.cm.accel_components        << "\n";
    file << "show_trail="              << (config.cm.show_trail              ? "1" : "0") << "\n";
    file << "trail_duration="          << config.cm.trail_duration          << "\n";
    file << "show_xcom_line="          << (config.cm.show_xcom_line          ? "1" : "0") << "\n";
    file << "show_support_line="       << (config.cm.show_support_line       ? "1" : "0") << "\n";
    file << "\n";

    file << "[Physics]\n";
    file << "gravity_enabled="  << (config.physics.gravity_enabled  ? "1" : "0") << "\n";
    file << "gravity="          << config.physics.gravity          << "\n";
    file << "accel="            << config.physics.accel            << "\n";
    file << "walk_max_speed="   << config.physics.walk_max_speed   << "\n";
    file << "floor_friction="   << config.physics.floor_friction   << "\n";
    file << "hold_speed="       << config.physics.hold_speed       << "\n";
    file << "stop_speed="       << config.physics.stop_speed       << "\n";
    file << "spring_enabled=" << (config.physics.spring_enabled ? "1" : "0") << "\n";
    file << "vy_max="        << config.physics.vy_max        << "\n";
    file << "d_soft="        << config.physics.d_soft        << "\n";
    file << "vy_tau="        << config.physics.vy_tau        << "\n";
    file << "jump_impulse="     << config.physics.jump_impulse     << "\n\n";

    file << "[TerrainSampling]\n";
    file << "w_back="     << config.terrain_sampling.w_back     << "\n";
    file << "w_fwd="      << config.terrain_sampling.w_fwd      << "\n";
    file << "t_look="     << config.terrain_sampling.t_look     << "\n";
    file << "tau_slide="  << config.terrain_sampling.tau_slide  << "\n\n";

    file << "[Terrain]\n";
    file << "enabled="     << (config.terrain.enabled ? "1" : "0") << "\n";
    file << "seed="        << config.terrain.seed        << "\n";
    file << "seg_min="     << config.terrain.seg_min     << "\n";
    file << "seg_max="     << config.terrain.seg_max     << "\n";
    file << "angle_small=" << config.terrain.angle_small << "\n";
    file << "angle_large=" << config.terrain.angle_large << "\n";
    file << "large_prob="  << config.terrain.large_prob  << "\n";
    file << "slope_max="   << config.terrain.slope_max   << "\n";
    file << "height_min="  << config.terrain.height_min  << "\n";
    file << "height_max="  << config.terrain.height_max  << "\n\n";

    file << "[Particles]\n";
    file << "enabled="        << (config.particles.enabled ? "1" : "0") << "\n";
    file << "dust_enabled="   << (config.particles.dust_enabled ? "1" : "0") << "\n";
    file << "impact_enabled=" << (config.particles.impact_enabled ? "1" : "0") << "\n";
    file << "slide_enabled="  << (config.particles.slide_enabled ? "1" : "0") << "\n";
    file << "landing_enabled="<< (config.particles.landing_enabled ? "1" : "0") << "\n";
    file << "dust_burst_count=" << config.particles.dust_burst_count << "\n";
    file << "dust_radius_px=" << config.particles.dust_radius_px << "\n";
    file << "dust_alpha="     << config.particles.dust_alpha << "\n";
    file << "dust_lifetime_s="<< config.particles.dust_lifetime_s << "\n";
    file << "dust_speed_mps=" << config.particles.dust_speed_mps << "\n";
    file << "slide_emit_interval_s=" << config.particles.slide_emit_interval_s << "\n";
    file << "landing_burst_scale="   << config.particles.landing_burst_scale << "\n\n";

    file << "[Audio]\n";
    file << "footstep_volume=" << config.audio.footstep_volume << "\n";
    file << "music_volume="    << config.audio.music_volume    << "\n";
    file << "music_enabled="   << (config.audio.music_enabled ? "1" : "0") << "\n";
    file << "music_track="     << config.audio.music_track     << "\n\n";

    file << "[Standing]\n";
    file << "d_pref=" << config.standing.d_pref << "\n";
    file << "d_min="  << config.standing.d_min  << "\n";
    file << "d_max="    << config.standing.d_max    << "\n";
    file << "eps_v="    << config.standing.eps_v    << "\n\n";

    file << "[Step]\n";
    file << "h_clear_ratio=" << config.step.h_clear_ratio << "\n\n";

    file << "[Run]\n";
    file << "max_speed="        << config.run.max_speed        << "\n";
    file << "accel_factor="     << config.run.accel_factor     << "\n";
    file << "step_speed="       << config.run.step_speed       << "\n";
    file << "stability_margin=" << config.run.stability_margin << "\n";
    file << "max_step_L="       << config.run.max_step_L       << "\n";
    file << "d_rear_max="       << config.run.d_rear_max       << "\n";
    file << "xcom_scale="       << config.run.xcom_scale       << "\n";
    file << "stride_len="       << config.run.stride_len       << "\n";
    file << "stride_len_min="   << config.run.stride_len_min   << "\n";
    file << "cadence_spm_min="  << config.run.cadence_spm_min  << "\n";
    file << "cadence_spm_max="  << config.run.cadence_spm_max  << "\n";
    file << "theta_max_deg="    << config.run.theta_max_deg    << "\n";
    file << "leg_flex_coeff="   << config.run.leg_flex_coeff   << "\n";
    file << "bob_scale="        << config.run.bob_scale        << "\n";
    file << "bob_amp="          << config.run.bob_amp          << "\n";
    file << "h_clear_ratio="    << config.run.h_clear_ratio    << "\n";
    file << "h_clear_min_ratio="<< config.run.h_clear_min_ratio<< "\n";
    file << "blend_tau="        << config.run.blend_tau        << "\n\n";

    file << "[Jump]\n";
    file << "preload_dur_run="          << config.jump.preload_dur_run          << "\n";
    file << "preload_dur_walk="         << config.jump.preload_dur_walk         << "\n";
    file << "preload_dur_stand="        << config.jump.preload_dur_stand        << "\n";
    file << "preload_depth_run="        << config.jump.preload_depth_run        << "\n";
    file << "preload_depth_walk="       << config.jump.preload_depth_walk       << "\n";
    file << "preload_depth_stand="      << config.jump.preload_depth_stand      << "\n";
    file << "tuck_height_ratio="        << config.jump.tuck_height_ratio        << "\n";
    file << "landing_dur_jump="         << config.jump.landing_dur_jump         << "\n";
    file << "landing_dur_walk="         << config.jump.landing_dur_walk         << "\n";
    file << "landing_boost_base_jump="  << config.jump.landing_boost_base_jump  << "\n";
    file << "landing_boost_scale_jump=" << config.jump.landing_boost_scale_jump << "\n";
    file << "landing_boost_base_walk="  << config.jump.landing_boost_base_walk  << "\n";
    file << "landing_boost_scale_walk=" << config.jump.landing_boost_scale_walk << "\n\n";

    file << "[Walk]\n";
    file << "eps_step="            << config.walk.eps_step            << "\n";
    file << "xcom_scale="          << config.walk.xcom_scale          << "\n";
    file << "d_rear_max="          << config.walk.d_rear_max          << "\n";
    file << "max_step_L="          << config.walk.max_step_L          << "\n";
    file << "max_speed_drop="      << config.walk.max_speed_drop      << "\n";
    file << "max_slope_drop="      << config.walk.max_slope_drop      << "\n";
    file << "downhill_crouch_max=" << config.walk.downhill_crouch_max << "\n";
    file << "downhill_crouch_tau=" << config.walk.downhill_crouch_tau << "\n";
    file << "downhill_relax_tau="  << config.walk.downhill_relax_tau  << "\n";
    file << "downhill_step_bonus=" << config.walk.downhill_step_bonus << "\n";
    file << "step_speed="          << config.walk.step_speed          << "\n";
    file << "stability_margin="           << config.walk.stability_margin           << "\n";
    file << "double_support_time=" << config.walk.double_support_time << "\n";
    file << "cm_height_offset="    << config.walk.cm_height_offset    << "\n";
    file << "leg_flex_coeff="      << config.walk.leg_flex_coeff      << "\n";
    file << "bob_scale="           << config.walk.bob_scale           << "\n";
    file << "bob_amp="             << config.walk.bob_amp             << "\n";
    file << "h_clear_slope_factor="<< config.walk.h_clear_slope_factor<< "\n";
    file << "h_clear_speed_factor="<< config.walk.h_clear_speed_factor<< "\n";
    file << "h_clear_min_ratio="   << config.walk.h_clear_min_ratio   << "\n";

    return true;
}
