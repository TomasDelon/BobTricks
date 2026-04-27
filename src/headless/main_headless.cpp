#include <cstdio>
#include <iostream>
#include <map>
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
            for (const std::pair<const std::string, ScenarioFactory>& entry : scenarioLibrary())
                fprintf(stdout, "%s\n", entry.first.c_str());
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
    ConfigIO::load(CONFIG_PATH, config);  // utilise silencieusement les valeurs par défaut si le fichier manque

    const std::map<std::string, ScenarioFactory>& lib = scenarioLibrary();

    // ── --all : exécuter tous les scénarios enregistrés ─────────────────────
    if (run_all) {
        int n_pass = 0, n_fail = 0;
        for (const std::pair<const std::string, ScenarioFactory>& entry : lib) {
            // Nouvelle copie de la config pour chaque scénario : SimulationCore::reset()
            // peut modifier config.terrain.seed, donc chaque exécution doit repartir
            // d'un état propre.
            AppConfig   run_cfg = config;
            ScenarioDef def     = entry.second(run_cfg);
            fprintf(stderr, "[headless] scenario=%-20s  duration=%.1fs\n",
                    entry.first.c_str(), def.duration_s);

            // Ignorer le CSV vers l'équivalent de /dev/null (flux nul) : seules les assertions comptent.
            std::ostream null_out(nullptr);
            const bool passed = runScenario(def, run_cfg, null_out, std::cerr);
            fprintf(stderr, "%s  %s\n\n", passed ? "PASS" : "FAIL", entry.first.c_str());
            passed ? ++n_pass : ++n_fail;
        }
        fprintf(stderr, "Results: %d PASS  %d FAIL\n", n_pass, n_fail);
        return (n_fail == 0) ? 0 : 1;
    }

    // ── --scenario : exécuter un seul scénario, CSV vers la sortie standard ─
    if (scenario_name.empty())
        scenario_name = "walk_3s";   // valeur par défaut

    const std::map<std::string, ScenarioFactory>::const_iterator it = lib.find(scenario_name);
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
