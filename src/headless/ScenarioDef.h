#pragma once

#include <functional>
#include <string>

#include "core/simulation/SimState.h"
#include "core/simulation/InputFrame.h"
#include "core/telemetry/TelemetryRecorder.h"

/** @brief Définition autonome d'un scénario headless. */
struct ScenarioDef {
    std::string  name;
    double       duration_s  = 1.0;
    ScenarioInit init;

    std::function<InputFrame(double /*sim_time*/)>   input_fn;
    std::function<void(TelemetryRecorder&)>          setup_asserts;
};
