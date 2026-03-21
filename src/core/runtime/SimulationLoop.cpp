#include "core/runtime/SimulationLoop.h"

#include <algorithm>

SimulationLoop::SimulationLoop(const Config& config)
    : m_config(config)
{}

void SimulationLoop::reset()
{
    m_accumulator_s        = 0.0;
    m_pending_single_steps = 0;
    m_total_step_count     = 0;
}

void SimulationLoop::setPaused(bool paused)   { m_paused = paused; }
void SimulationLoop::togglePaused()            { m_paused = !m_paused; }
bool SimulationLoop::isPaused() const          { return m_paused; }

void SimulationLoop::requestSingleStep(std::uint32_t count)
{
    m_pending_single_steps += count;
}

void SimulationLoop::setTotalStepCount(std::uint64_t count) { m_total_step_count = count; }

void SimulationLoop::setTimeScale(double time_scale)
{
    m_time_scale = std::max(0.0, time_scale);
}

double        SimulationLoop::getFixedDt()        const { return m_config.fixed_dt_s; }
double        SimulationLoop::getTimeScale()       const { return m_time_scale; }
std::uint64_t SimulationLoop::getTotalStepCount()  const { return m_total_step_count; }
double        SimulationLoop::getSimulationTime()  const { return static_cast<double>(m_total_step_count) * m_config.fixed_dt_s; }

std::uint32_t SimulationLoop::runFrame(double real_dt_s, const std::function<void(double)>& step_fn)
{
    std::uint32_t steps = 0;

    if (!m_paused) {
        const double clamped = std::clamp(real_dt_s, 0.0, m_config.max_frame_dt_s);
        const double scaled  = clamped * m_time_scale;
        m_accumulator_s = std::min(m_accumulator_s + scaled, m_config.max_accumulator_s);

        while (m_accumulator_s >= m_config.fixed_dt_s) {
            step_fn(m_config.fixed_dt_s);
            m_accumulator_s -= m_config.fixed_dt_s;
            ++steps;
            ++m_total_step_count;
        }
    }

    while (m_paused && m_pending_single_steps > 0) {
        step_fn(m_config.fixed_dt_s);
        --m_pending_single_steps;
        ++steps;
        ++m_total_step_count;
    }

    return steps;
}
