#include "headless/ScenarioRunner.h"

#include <ostream>

#include "core/simulation/SimulationCore.h"
#include "core/telemetry/TelemetryRecorder.h"

bool runScenario(const ScenarioDef& def,
                 AppConfig&         config,
                 std::ostream&      csv_out,
                 std::ostream&      report_out)
{
    SimulationCore    core(config);
    TelemetryRecorder rec;

    core.reset(def.init);

    if (def.setup_asserts)
        def.setup_asserts(rec);

    // Record the initial state (t=0) before any integration.
    // NOTE: feet are not yet bootstrapped at this point — foot positions,
    // support, xcom and mos will be zero.  The row's cm position and velocity
    // do reflect the ScenarioInit values exactly.  Bootstrap happens inside
    // the first step() call below.
    rec.record(core.state());

    const double dt = config.sim_loop.fixed_dt_s;

    // Step loop — deterministic fixed timestep, no real-time throttle.
    // sim_time is read from core.state() so the InputFrame always gets
    // the time that was current at the START of the step being built.
    while (core.time() < def.duration_s) {
        const InputFrame input = def.input_fn
                               ? def.input_fn(core.time())
                               : InputFrame{};
        core.step(dt, input);
        rec.record(core.state());
    }

    rec.writeCsv(csv_out);

    if (rec.rows().empty() || rec.rows().front().t == rec.rows().back().t) {
        // degenerate run — report but don't crash
        report_out << "WARN  empty or single-row run for scenario: " << def.name << '\n';
    }

    return rec.runAssertions(report_out);
}
