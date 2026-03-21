#pragma once

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

#include "core/telemetry/TelemetryRow.h"
#include "core/simulation/SimState.h"

// Records simulation telemetry and evaluates pass/fail assertions.
//
// Usage pattern:
//   TelemetryRecorder rec;
//   rec.addAssertion("heel_strikes >= 2", [](auto& rows){ ... });
//   while (running) {
//       core.step(dt, input);
//       rec.record(core.state());
//   }
//   rec.writeCsv(std::cout);
//   bool ok = rec.runAssertions(std::cerr);
//
// The recorder is decoupled from SimulationCore — it is the caller's
// responsibility to call record() after each step().
class TelemetryRecorder
{
public:
    // Append one row from the current simulation state.
    void record(const SimState& state);

    // Write CSV to any ostream (file, stdout, stringstream).
    // First line is the frozen header; subsequent lines are data rows.
    void writeCsv(std::ostream& out) const;

    // Read-only access to all recorded rows.
    const std::vector<TelemetryRow>& rows() const { return m_rows; }

    // Discard all rows and assertions.
    void clear();

    // ── Assertions ───────────────────────────────────────────────────────────

    // Register an assertion.  fn receives the full row vector; returns true = PASS.
    // Assertions are evaluated by runAssertions(), not by record().
    void addAssertion(const std::string& name,
                      std::function<bool(const std::vector<TelemetryRow>&)> fn);

    // Evaluate all registered assertions.
    // Writes "PASS  <name>" or "FAIL  <name>" for each to `report`.
    // Returns true if every assertion passes.
    bool runAssertions(std::ostream& report) const;

private:
    std::vector<TelemetryRow> m_rows;

    struct Assertion {
        std::string name;
        std::function<bool(const std::vector<TelemetryRow>&)> check;
    };
    std::vector<Assertion> m_asserts;
};
