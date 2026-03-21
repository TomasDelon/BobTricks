#pragma once

#include <iosfwd>

#include "config/AppConfig.h"
#include "headless/ScenarioDef.h"

// Runs one scenario and writes telemetry + assertion report.
//
// Returns true if all assertions pass (or none were registered).
// CSV is written to csv_out; assertion results to report_out.
//
// config is passed by non-const reference because SimulationCore stores
// a live reference to it (sliders / terrain seed may be mutated by reset).
// For deterministic test runs, do not mutate config after calling run().
bool runScenario(const ScenarioDef& def,
                 AppConfig&         config,
                 std::ostream&      csv_out,
                 std::ostream&      report_out);
