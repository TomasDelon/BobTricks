#pragma once

#include <cstdint>
#include <functional>

class SimulationLoop
{
public:
    struct Config {
        double fixed_dt_s       = 1.0 / 60.0;
        double max_frame_dt_s   = 0.25;
        double max_accumulator_s = 0.25;
    };

    SimulationLoop() = default;
    explicit SimulationLoop(const Config& config);

    void reset();

    void setPaused(bool paused);
    void togglePaused();
    bool isPaused() const;

    void requestSingleStep(std::uint32_t count = 1);
    void setTotalStepCount(std::uint64_t count);
    void setTimeScale(double time_scale);

    double        getFixedDt()          const;
    double        getTimeScale()        const;
    std::uint64_t getTotalStepCount()   const;
    double        getSimulationTime()   const;

    std::uint32_t runFrame(double real_dt_s, const std::function<void(double)>& step_fn);

private:
    Config        m_config;
    double        m_accumulator_s        = 0.0;
    double        m_time_scale           = 1.0;
    bool          m_paused               = false;
    std::uint32_t m_pending_single_steps = 0;
    std::uint64_t m_total_step_count     = 0;
};
