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

    // Enregistre l'état initial (t=0) avant toute intégration.
    // À cet instant les pieds ne sont pas encore bootstrapés : positions de
    // pieds, support, centre de masse extrapolé et marge de stabilité valent
    // encore zéro. En revanche, la position et la vitesse du CM reflètent bien
    // exactement les valeurs de `ScenarioInit`. Le bootstrap est fait dans le
    // premier appel à `step()` ci-dessous.
    rec.record(core.state());

    const double dt = config.sim_loop.fixed_dt_s;

    // Boucle de pas déterministe à dt fixe, sans contrainte temps réel.
    // `sim_time` est lu depuis `core.state()` pour que l'`InputFrame` reçoive
    // toujours le temps courant au DÉBUT du pas en cours de construction.
    while (core.time() < def.duration_s) {
        const InputFrame input = def.input_fn
                               ? def.input_fn(core.time())
                               : InputFrame{};
        core.step(dt, input);
        rec.record(core.state());
    }

    rec.writeCsv(csv_out);

    if (rec.rows().empty() || rec.rows().front().t == rec.rows().back().t) {
        // Exécution dégénérée : on le signale sans faire échouer brutalement.
        report_out << "WARN  empty or single-row run for scenario: " << def.name << '\n';
    }

    return rec.runAssertions(report_out);
}
