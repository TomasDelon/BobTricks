#include "config/ConfigIO.h"

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

    while (std::getline(file, line)) {
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

        if (section == "SimLoop") {
            if      (key == "max_fps")    config.sim_loop.max_fps    = std::stoi(value);
            else if (key == "fixed_dt_s") config.sim_loop.fixed_dt_s = std::stod(value);
            else if (key == "time_scale") config.sim_loop.time_scale = std::stod(value);
        }
        else if (section == "Camera") {
            if      (key == "zoom")     config.camera.zoom     = std::stod(value);
            else if (key == "follow_x") config.camera.follow_x = (value == "1" || value == "true");
            else if (key == "follow_y") config.camera.follow_y = (value == "1" || value == "true");
            else if (key == "smooth_x") config.camera.smooth_x = std::stod(value);
            else if (key == "smooth_y") config.camera.smooth_y = std::stod(value);
        }
        else if (section == "Character") {
            if      (key == "body_height_m")   config.character.body_height_m   = std::stod(value);
            else if (key == "body_mass_kg")    config.character.body_mass_kg    = std::stod(value);
            else if (key == "cm_pelvis_ratio") config.character.cm_pelvis_ratio = std::stod(value);
        }
        else if (section == "Reconstruction") {
            if      (key == "facing_tau")    config.reconstruction.facing_tau    = std::stod(value);
            else if (key == "facing_eps")    config.reconstruction.facing_eps    = std::stod(value);
            else if (key == "walk_eps")      config.reconstruction.walk_eps      = std::stod(value);
            else if (key == "theta_max_deg") config.reconstruction.theta_max_deg = std::stod(value);
            else if (key == "v_ref")         config.reconstruction.v_ref         = std::stod(value);
        }
        else if (section == "CM") {
            if      (key == "show_ground_reference") config.cm.show_ground_reference = (value == "1" || value == "true");
            else if (key == "show_projection_line") config.cm.show_projection_line = (value == "1" || value == "true");
            else if (key == "show_projection_dot")  config.cm.show_projection_dot  = (value == "1" || value == "true");
            else if (key == "show_target_height_tick") config.cm.show_target_height_tick = (value == "1" || value == "true");
            else if (key == "velocity_components")  config.cm.velocity_components  = std::stoi(value);
            else if (key == "accel_components")     config.cm.accel_components     = std::stoi(value);
            else if (key == "show_trail")                config.cm.show_trail               = (value == "1" || value == "true");
            else if (key == "trail_duration")            config.cm.trail_duration           = std::stod(value);
            else if (key == "show_xcom_line")            config.cm.show_xcom_line           = (value == "1" || value == "true");
            else if (key == "show_support_line")         config.cm.show_support_line        = (value == "1" || value == "true");
            else if (key == "show_planted_feet_color")   config.cm.show_planted_feet_color  = (value == "1" || value == "true");
        }
        else if (section == "Physics") {
            if      (key == "gravity_enabled")        config.physics.gravity_enabled        = (value == "1" || value == "true");
            else if (key == "gravity")                config.physics.gravity                = std::stod(value);
            else if (key == "air_friction_enabled")   config.physics.air_friction_enabled   = (value == "1" || value == "true");
            else if (key == "air_friction")           config.physics.air_friction           = std::stod(value);
            else if (key == "floor_friction_enabled") config.physics.floor_friction_enabled = (value == "1" || value == "true");
            else if (key == "floor_friction")         config.physics.floor_friction         = std::stod(value);
            else if (key == "spring_enabled")         config.physics.spring_enabled         = (value == "1" || value == "true");
            else if (key == "spring_stiffness")       config.physics.spring_stiffness       = std::stod(value);
            else if (key == "spring_damping")         config.physics.spring_damping         = std::stod(value);
            else if (key == "move_force")             config.physics.move_force             = std::stod(value);
            else if (key == "jump_impulse")           config.physics.jump_impulse           = std::stod(value);
        }
        else if (section == "Terrain") {
            if      (key == "enabled")     config.terrain.enabled     = (value == "1" || value == "true");
            else if (key == "seed")        config.terrain.seed        = std::stoi(value);
            else if (key == "seg_min")     config.terrain.seg_min     = std::stod(value);
            else if (key == "seg_max")     config.terrain.seg_max     = std::stod(value);
            else if (key == "angle_small") config.terrain.angle_small = std::stod(value);
            else if (key == "angle_large") config.terrain.angle_large = std::stod(value);
            else if (key == "large_prob")  config.terrain.large_prob  = std::stod(value);
            else if (key == "slope_max")   config.terrain.slope_max   = std::stod(value);
            else if (key == "height_min")  config.terrain.height_min  = std::stod(value);
            else if (key == "height_max")  config.terrain.height_max  = std::stod(value);
        }
        else if (section == "Standing") {
            if      (key == "d_pref")   config.standing.d_pref   = std::stod(value);
            else if (key == "d_min")    config.standing.d_min    = std::stod(value);
            else if (key == "d_max")    config.standing.d_max    = std::stod(value);
            else if (key == "k_leg")    config.standing.k_leg    = std::stod(value);
            else if (key == "eps_v")    config.standing.eps_v    = std::stod(value);
            else if (key == "k_crouch") config.standing.k_crouch = std::stod(value);
        }
        else if (section == "Balance") {
            if      (key == "h_ref_override")     config.balance.h_ref_override     = std::stod(value);
            else if (key == "mos_step_threshold") config.balance.mos_step_threshold = std::stod(value);
        }
        else if (section == "Step") {
            if      (key == "h_clear_ratio")    config.step.h_clear_ratio    = std::stod(value);
            else if (key == "T_min")            config.step.T_min            = std::stod(value);
            else if (key == "T_max")            config.step.T_max            = std::stod(value);
            else if (key == "dist_coeff")       config.step.dist_coeff       = std::stod(value);
            else if (key == "d_max_correction") config.step.d_max_correction = std::stod(value);
            else if (key == "k_bob")            config.step.k_bob            = std::stod(value);
            else if (key == "v_ref_bob")        config.step.v_ref_bob        = std::stod(value);
        }
        else if (section == "Walk") {
            if      (key == "k_trigger") config.walk.k_trigger = std::stod(value);
            else if (key == "k_step")    config.walk.k_step    = std::stod(value);
            else if (key == "T_ant")     config.walk.T_ant     = std::stod(value);
        }
    }

    return true;
}

bool ConfigIO::save(const std::string& path, const AppConfig& config)
{
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "; BobTricks V4 — configuration file\n\n";

    file << "[SimLoop]\n";
    file << "max_fps="    << config.sim_loop.max_fps    << "\n";
    file << "fixed_dt_s=" << config.sim_loop.fixed_dt_s << "\n";
    file << "time_scale=" << config.sim_loop.time_scale << "\n";
    file << "\n";

    file << "[Camera]\n";
    file << "zoom="     << config.camera.zoom                     << "\n";
    file << "follow_x=" << (config.camera.follow_x ? "1" : "0")  << "\n";
    file << "follow_y=" << (config.camera.follow_y ? "1" : "0")  << "\n";
    file << "smooth_x=" << config.camera.smooth_x                 << "\n";
    file << "smooth_y=" << config.camera.smooth_y                 << "\n";
    file << "\n";

    file << "[Character]\n";
    file << "body_height_m="   << config.character.body_height_m   << "\n";
    file << "body_mass_kg="    << config.character.body_mass_kg    << "\n";
    file << "cm_pelvis_ratio=" << config.character.cm_pelvis_ratio << "\n";
    file << "\n";

    file << "[Reconstruction]\n";
    file << "facing_tau="    << config.reconstruction.facing_tau    << "\n";
    file << "facing_eps="    << config.reconstruction.facing_eps    << "\n";
    file << "walk_eps="      << config.reconstruction.walk_eps      << "\n";
    file << "theta_max_deg=" << config.reconstruction.theta_max_deg << "\n";
    file << "v_ref="         << config.reconstruction.v_ref         << "\n";
    file << "\n";

    file << "[CM]\n";
    file << "show_ground_reference=" << (config.cm.show_ground_reference ? "1" : "0") << "\n";
    file << "show_projection_line=" << (config.cm.show_projection_line ? "1" : "0") << "\n";
    file << "show_projection_dot="  << (config.cm.show_projection_dot  ? "1" : "0") << "\n";
    file << "show_target_height_tick=" << (config.cm.show_target_height_tick ? "1" : "0") << "\n";
    file << "velocity_components="  << config.cm.velocity_components                << "\n";
    file << "accel_components="     << config.cm.accel_components                   << "\n";
    file << "show_trail="                << (config.cm.show_trail ? "1" : "0")              << "\n";
    file << "trail_duration="           << config.cm.trail_duration                        << "\n";
    file << "show_xcom_line="           << (config.cm.show_xcom_line ? "1" : "0")          << "\n";
    file << "show_support_line="        << (config.cm.show_support_line ? "1" : "0")       << "\n";
    file << "show_planted_feet_color="  << (config.cm.show_planted_feet_color ? "1" : "0") << "\n";
    file << "\n";

    file << "[Physics]\n";
    file << "gravity_enabled="        << (config.physics.gravity_enabled        ? "1" : "0") << "\n";
    file << "gravity="                << config.physics.gravity                              << "\n";
    file << "air_friction_enabled="   << (config.physics.air_friction_enabled   ? "1" : "0") << "\n";
    file << "air_friction="           << config.physics.air_friction                         << "\n";
    file << "floor_friction_enabled=" << (config.physics.floor_friction_enabled ? "1" : "0") << "\n";
    file << "floor_friction="         << config.physics.floor_friction                       << "\n";
    file << "spring_enabled="         << (config.physics.spring_enabled         ? "1" : "0") << "\n";
    file << "spring_stiffness="       << config.physics.spring_stiffness                     << "\n";
    file << "spring_damping="         << config.physics.spring_damping                       << "\n";
    file << "move_force="             << config.physics.move_force                           << "\n";
    file << "jump_impulse="           << config.physics.jump_impulse                         << "\n";
    file << "\n";

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
    file << "height_max="  << config.terrain.height_max  << "\n";
    file << "\n";

    file << "[Standing]\n";
    file << "d_pref="   << config.standing.d_pref   << "\n";
    file << "d_min="    << config.standing.d_min    << "\n";
    file << "d_max="    << config.standing.d_max    << "\n";
    file << "k_leg="    << config.standing.k_leg    << "\n";
    file << "eps_v="    << config.standing.eps_v    << "\n";
    file << "k_crouch=" << config.standing.k_crouch << "\n";
    file << "\n";

    file << "[Balance]\n";
    file << "h_ref_override="     << config.balance.h_ref_override     << "\n";
    file << "mos_step_threshold=" << config.balance.mos_step_threshold << "\n";
    file << "\n";

    file << "[Step]\n";
    file << "h_clear_ratio="    << config.step.h_clear_ratio    << "\n";
    file << "T_min="            << config.step.T_min            << "\n";
    file << "T_max="            << config.step.T_max            << "\n";
    file << "dist_coeff="       << config.step.dist_coeff       << "\n";
    file << "d_max_correction=" << config.step.d_max_correction << "\n";
    file << "k_bob="            << config.step.k_bob            << "\n";
    file << "v_ref_bob="        << config.step.v_ref_bob        << "\n";
    file << "\n";

    file << "[Walk]\n";
    file << "k_trigger=" << config.walk.k_trigger << "\n";
    file << "k_step="    << config.walk.k_step    << "\n";
    file << "T_ant="     << config.walk.T_ant     << "\n";

    return true;
}
