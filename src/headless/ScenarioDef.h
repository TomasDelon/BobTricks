#pragma once

#include <functional>
#include <string>

#include "core/simulation/SimState.h"
#include "core/simulation/InputFrame.h"
#include "core/telemetry/TelemetryRecorder.h"

// A self-contained scenario definition.
//
// The ScenarioRunner creates a SimulationCore from config + init,
// steps it for duration_s seconds, records telemetry, then evaluates
// all assertions registered by setup_asserts.
//
// input_fn  : called each tick with the current sim_time; returns the
//             InputFrame for that step.  nullptr = idle (no input).
// setup_asserts : called once before the run; registers assertions on the
//                 TelemetryRecorder.  nullptr = no assertions (always PASS).
struct ScenarioDef {
    std::string  name;
    double       duration_s  = 1.0;
    ScenarioInit init;

    std::function<InputFrame(double /*sim_time*/)>   input_fn;
    std::function<void(TelemetryRecorder&)>          setup_asserts;
};
