#pragma once

#include <functional>
#include <string>

#include "core/simulation/SimState.h"
#include "core/simulation/InputFrame.h"
#include "core/telemetry/TelemetryRecorder.h"

/**
 * @brief Définition autonome d'un scénario headless.
 *
 * Un scénario combine un état initial, une loi d'entrée pilotée par le temps
 * simulé et un ensemble d'assertions exécutées à la fin du run.
 */
struct ScenarioDef {
    /** @brief Nom lisible du scénario. */
    std::string  name;
    /** @brief Durée totale du scénario en secondes simulées. */
    double       duration_s  = 1.0;
    /** @brief Conditions initiales appliquées avant la première frame. */
    ScenarioInit init;

    /** @brief Fonction d'entrée évaluée à chaque pas de simulation. */
    std::function<InputFrame(double /*sim_time*/)>   input_fn;
    /** @brief Fonction enregistrant les assertions finales à exécuter. */
    std::function<void(TelemetryRecorder&)>          setup_asserts;
};
