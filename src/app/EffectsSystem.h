#pragma once

#include <deque>

#include "config/AppConfig.h"
#include "core/simulation/SimState.h"
#include "render/DustParticle.h"

class EffectsSystem
{
public:
    void update(const SimState& state, const ParticlesConfig& config, double sim_time);
    void clear();

    const std::deque<DustParticle>& dustParticles() const { return m_dust_particles; }

private:
    void emitFootDust(const FootState& foot,
                      const ParticlesConfig& config,
                      double sim_time,
                      double burst_scale,
                      double tangent_bias,
                      double vertical_scale);
    void emitSlideDust(const FootState& foot,
                       const ParticlesConfig& config,
                       double sim_time,
                       double tangent_dir);
    void pruneDustParticles(double sim_time);

    std::deque<DustParticle> m_dust_particles;
};
