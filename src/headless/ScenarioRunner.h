#pragma once

#include <iosfwd>

#include "config/AppConfig.h"
#include "headless/ScenarioDef.h"

/**
 * @brief Exécute un scénario headless, écrit la télémétrie et évalue les assertions.
 * @param def        Définition complète du scénario à jouer.
 * @param config     Configuration mutable utilisée pour construire le noyau.
 * @param csv_out    Flux recevant la télémétrie CSV.
 * @param report_out Flux recevant le rapport textuel des assertions.
 * @return `true` si toutes les assertions du scénario passent.
 */
bool runScenario(const ScenarioDef& def,
                 AppConfig&         config,
                 std::ostream&      csv_out,
                 std::ostream&      report_out);
