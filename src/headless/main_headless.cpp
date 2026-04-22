#include <cstdio>
#include <iostream>
#include <string>

#include "config/AppConfig.h"
#include "config/ConfigIO.h"
#include "core/simulation/SimVerbosity.h"
#include "headless/ScenarioLibrary.h"
#include "headless/ScenarioRunner.h"

static constexpr const char* CONFIG_PATH = "data/config.ini";

int main(int argc, char** argv)
{
    std::string scenario_name;
    bool        run_all = false;
    bool        quiet   = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--scenario" && i + 1 < argc)
            scenario_name = argv[++i];
        else if (arg == "--all"   || arg == "-a")
            run_all = true;
        else if (arg == "--quiet" || arg == "-q")
            quiet = true;
        else if (arg == "--list"  || arg == "-l") {
            for (const auto& [name, _] : scenarioLibrary())
                fprintf(stdout, "%s\n", name.c_str());
            return 0;
        }
        else if (arg == "--help"  || arg == "-h") {
            fprintf(stderr,
                "Usage: bobtricks_headless [options]\n"
                "Options:\n"
                "  --scenario <name>   run one scenario (CSV to stdout, report to stderr)\n"
                "  --all / -a          run all scenarios; exit 0 if all pass, 1 if any fail\n"
                "  --list / -l         print available scenario names and exit\n"
                "  --quiet / -q        suppress core debug logs (Bootstrap, stepping, ...)\n"
                "  --help  / -h        show this message\n");
            return 0;
        }
    }

    if (quiet) g_sim_verbose = false;

    AppConfig config;
    ConfigIO::load(CONFIG_PATH, config);  // silently uses defaults if file missing

    const auto& lib = scenarioLibrary();

    // ── --all: run every registered scenario ────────────────────────────────
    if (run_all) {
        int n_pass = 0, n_fail = 0;
        for (const auto& [name, factory] : lib) {
            // Fresh config copy per scenario: SimulationCore::reset() may mutate
            // config.terrain.seed, so each run must start from a clean state.
            AppConfig   run_cfg = config;
            ScenarioDef def     = factory(run_cfg);
            fprintf(stderr, "[headless] scenario=%-20s  duration=%.1fs\n",
                    name.c_str(), def.duration_s);

            // Discard CSV to /dev/null equivalent (null stream) — only assertions matter.
            std::ostream null_out(nullptr);
            const bool passed = runScenario(def, run_cfg, null_out, std::cerr);
            fprintf(stderr, "%s  %s\n\n", passed ? "PASS" : "FAIL", name.c_str());
            passed ? ++n_pass : ++n_fail;
        }
        fprintf(stderr, "Results: %d PASS  %d FAIL\n", n_pass, n_fail);
        return (n_fail == 0) ? 0 : 1;
    }

    // ── --scenario: run one scenario, CSV to stdout ──────────────────────────
    if (scenario_name.empty())
        scenario_name = "walk_3s";   // default

    const auto it = lib.find(scenario_name);
    if (it == lib.end()) {
        fprintf(stderr, "Unknown scenario: %s\n", scenario_name.c_str());
        fprintf(stderr, "Run --list to see available scenarios.\n");
        return 1;
    }

    ScenarioDef def = it->second(config);
    fprintf(stderr, "[headless] scenario=%s  duration=%.1fs\n",
            def.name.c_str(), def.duration_s);

    const bool passed = runScenario(def, config, std::cout, std::cerr);
    return passed ? 0 : 1;
}
