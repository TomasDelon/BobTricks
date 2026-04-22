#include "app/EffectsSystem.h"

#include <algorithm>
#include <cmath>

namespace {

double hash01(int seed)
{
    const double x = std::sin(static_cast<double>(seed) * 12.9898 + 78.233) * 43758.5453123;
    return x - std::floor(x);
}

} // namespace

void EffectsSystem::update(const SimState& state,
                           const CharacterConfig& char_cfg,
                           const ParticlesConfig& config,
                           double sim_time)
{
    if (state.character.feet_initialized && config.enabled && config.dust_enabled) {
        const double move_sign = (std::abs(state.cm.velocity.x) > 0.05)
                               ? ((state.cm.velocity.x > 0.0) ? 1.0 : -1.0)
                               : state.character.facing;

        if (config.impact_enabled) {
            if (state.events.left_touchdown)
                emitFootDust(state.character.foot_left, char_cfg, config, sim_time, 1.2, move_sign, 0.9);
            if (state.events.right_touchdown)
                emitFootDust(state.character.foot_right, char_cfg, config, sim_time, 1.2, move_sign, 0.9);
        }

        if (config.landing_enabled && state.events.landed_from_jump) {
            const double landing_scale = std::max(1.0, config.landing_burst_scale);
            if (state.character.foot_left.on_ground)
                emitFootDust(state.character.foot_left, char_cfg, config, sim_time, landing_scale, move_sign, 1.3);
            if (state.character.foot_right.on_ground)
                emitFootDust(state.character.foot_right, char_cfg, config, sim_time, landing_scale, move_sign, 1.3);
        }

        if (config.slide_enabled && state.events.left_slide_active) {
            const double uphill_dir = (state.character.foot_left.ground_normal.x >= 0.0) ? -1.0 : 1.0;
            const double motion_dir = (state.cm.velocity.x >= 0.0) ? 1.0 : -1.0;
            const bool braking_on_slope = motion_dir != uphill_dir;
            const double tangent_dir = braking_on_slope ? motion_dir : uphill_dir;
            emitSlideDust(state.character.foot_left, char_cfg, config, sim_time, tangent_dir);
        }
        if (config.slide_enabled && state.events.right_slide_active) {
            const double uphill_dir = (state.character.foot_right.ground_normal.x >= 0.0) ? -1.0 : 1.0;
            const double motion_dir = (state.cm.velocity.x >= 0.0) ? 1.0 : -1.0;
            const bool braking_on_slope = motion_dir != uphill_dir;
            const double tangent_dir = braking_on_slope ? motion_dir : uphill_dir;
            emitSlideDust(state.character.foot_right, char_cfg, config, sim_time, tangent_dir);
        }
    }

    pruneDustParticles(sim_time);
}

void EffectsSystem::clear()
{
    m_dust_particles.clear();
}

void EffectsSystem::emitFootDust(const FootState& foot,
                                 const CharacterConfig& char_cfg,
                                 const ParticlesConfig& config,
                                 double sim_time,
                                 double burst_scale,
                                 double tangent_bias,
                                 double vertical_scale)
{
    const int burst_count = std::max(0, static_cast<int>(std::lround(
        static_cast<double>(config.dust_burst_count) * burst_scale)));
    if (burst_count == 0 || config.dust_lifetime_s <= 0.0) return;

    const Vec2 normal = foot.on_ground ? foot.ground_normal : Vec2{0.0, 1.0};
    Vec2 tangent{normal.y, -normal.x};
    if (tangent.length() < 1.0e-6) tangent = {1.0, 0.0};
    else tangent = tangent / tangent.length();
    const Vec2 up = (normal.length() < 1.0e-6) ? Vec2{0.0, 1.0} : (normal / normal.length());
    const double L = char_cfg.body_height_m / 5.0;
    const double foot_thickness = 0.08 * L;
    const Vec2 sole_origin = foot.pos - up * (0.5 * foot_thickness);

    const int seed_base = static_cast<int>(sim_time * 1000.0);
    for (int i = 0; i < burst_count; ++i) {
        const double spread = (hash01(seed_base + 31 * i) - 0.5) * 1.1;
        const double forward = 0.55 + 0.75 * hash01(seed_base + 67 * i);
        const double upward = vertical_scale * (0.25 + 0.70 * hash01(seed_base + 101 * i));
        DustParticle particle;
        particle.spawn_time = sim_time;
        particle.lifetime_s = config.dust_lifetime_s
                            * (0.75 + 0.5 * hash01(seed_base + 149 * i))
                            * std::max(0.8, burst_scale * 0.8);
        particle.pos = sole_origin + tangent * (0.02 * spread);
        particle.vel = tangent * (config.dust_speed_mps * (0.45 * spread + 0.55 * tangent_bias) * forward)
                     + up * (config.dust_speed_mps * 0.75 * upward);
        particle.radius_px = config.dust_radius_px
                           * static_cast<float>((0.7 + 0.8 * hash01(seed_base + 193 * i))
                           * std::max(0.85, burst_scale * 0.75));
        particle.alpha = config.dust_alpha
                       * static_cast<float>((0.55 + 0.45 * hash01(seed_base + 239 * i))
                       * std::max(0.85, burst_scale * 0.7));
        particle.stretch = static_cast<float>(1.15 + 0.65 * upward + 0.25 * std::abs(spread));
        particle.color_r = static_cast<unsigned char>(214 + 16 * hash01(seed_base + 271 * i));
        particle.color_g = static_cast<unsigned char>(194 + 18 * hash01(seed_base + 313 * i));
        particle.color_b = static_cast<unsigned char>(162 + 14 * hash01(seed_base + 347 * i));
        m_dust_particles.push_back(particle);
    }
}

void EffectsSystem::emitSlideDust(const FootState& foot,
                                  const CharacterConfig& char_cfg,
                                  const ParticlesConfig& config,
                                  double sim_time,
                                  double tangent_dir)
{
    if (config.dust_lifetime_s <= 0.0) return;

    const Vec2 normal = foot.on_ground ? foot.ground_normal : Vec2{0.0, 1.0};
    Vec2 tangent{normal.y, -normal.x};
    if (tangent.length() < 1.0e-6) tangent = {1.0, 0.0};
    else tangent = tangent / tangent.length();
    const Vec2 up = (normal.length() < 1.0e-6) ? Vec2{0.0, 1.0} : (normal / normal.length());
    const double L = char_cfg.body_height_m / 5.0;
    const double foot_thickness = 0.08 * L;
    const Vec2 sole_origin = foot.pos - up * (0.5 * foot_thickness);

    const int seed_base = static_cast<int>(sim_time * 1000.0) + 7000;
    for (int i = 0; i < 3; ++i) {
        const double jitter = (hash01(seed_base + 17 * i) - 0.5) * 0.6;
        DustParticle particle;
        particle.spawn_time = sim_time;
        particle.lifetime_s = config.dust_lifetime_s * (0.45 + 0.15 * hash01(seed_base + 29 * i));
        particle.pos = sole_origin + tangent * (0.016 * jitter);
        particle.vel = tangent * (config.dust_speed_mps * (0.45 * tangent_dir + 0.25 * jitter))
                     + up * (config.dust_speed_mps * (0.10 + 0.15 * hash01(seed_base + 43 * i)));
        particle.radius_px = config.dust_radius_px * static_cast<float>(0.45 + 0.20 * hash01(seed_base + 59 * i));
        particle.alpha = config.dust_alpha * static_cast<float>(0.35 + 0.15 * hash01(seed_base + 71 * i));
        particle.stretch = static_cast<float>(1.6 + 0.5 * hash01(seed_base + 89 * i));
        particle.color_r = 232;
        particle.color_g = static_cast<unsigned char>(210 + 10 * hash01(seed_base + 97 * i));
        particle.color_b = static_cast<unsigned char>(180 + 8 * hash01(seed_base + 109 * i));
        m_dust_particles.push_back(particle);
    }
}

void EffectsSystem::pruneDustParticles(double sim_time)
{
    while (!m_dust_particles.empty()) {
        const DustParticle& particle = m_dust_particles.front();
        if (sim_time - particle.spawn_time <= particle.lifetime_s) break;
        m_dust_particles.pop_front();
    }
}
