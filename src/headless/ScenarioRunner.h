#pragma once

/**
 * @file ScenarioRunner.h
 * @brief Exécution déterministe d'un scénario headless avec export télémétrie.
 */

#include <iosfwd>

#include "config/AppConfig.h"
#include "headless/ScenarioDef.h"

/**
 * @brief Exécute un scénario headless, écrit la télémétrie CSV et évalue les assertions.
 *
 * Cette fonction instancie un `SimulationCore`, exécute le scénario pas à pas
 * avec les entrées produites par `ScenarioDef::input_fn`, enregistre la
 * télémétrie dans un `TelemetryRecorder`, puis évalue les assertions définies
 * par `ScenarioDef::setup_asserts`.
 *
 * @param def        Définition du scénario à exécuter.
 * @param config     Configuration de l'application (non const — le noyau en a besoin par référence).
 * @param csv_out    Flux de sortie pour la télémétrie CSV.
 * @param report_out Flux de sortie pour le rapport d'assertions.
 * @return Vrai si toutes les assertions passent.
 */
bool runScenario(const ScenarioDef& def,
                 AppConfig&         config,
                 std::ostream&      csv_out,
                 std::ostream&      report_out);
