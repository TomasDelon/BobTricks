#pragma once

/**
 * @file ScenarioDef.h
 * @brief Définition autonome d'un scénario headless.
 */

#include <functional>
#include <string>

#include "core/simulation/SimState.h"
#include "core/simulation/InputFrame.h"
#include "core/telemetry/TelemetryRecorder.h"

/**
 * @brief Description complète d'un scénario headless exécutable par `runScenario()`.
 *
 * Un scénario encapsule :
 * - les conditions initiales (`init`) ;
 * - une fonction de génération d'entrée en fonction du temps (`input_fn`) ;
 * - des assertions optionnelles configurées sur le `TelemetryRecorder` (`setup_asserts`).
 */
struct ScenarioDef {
    std::string  name;              ///< Identifiant lisible du scénario.
    double       duration_s  = 1.0; ///< Durée totale de la simulation (s).
    ScenarioInit init;               ///< Conditions initiales du CM et du terrain.

    /** @brief Fonction appelée à chaque pas pour produire l'`InputFrame` correspondant. */
    std::function<InputFrame(double /*sim_time*/)>   input_fn;
    /** @brief Fonction appelée avant le début du scénario pour enregistrer les assertions. */
    std::function<void(TelemetryRecorder&)>          setup_asserts;
};
