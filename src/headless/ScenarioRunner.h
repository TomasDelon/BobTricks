#pragma once

#include <iosfwd>

#include "config/AppConfig.h"
#include "headless/ScenarioDef.h"

/**
 * @brief Exécute un scénario headless, écrit la télémétrie et évalue les assertions.
 */
bool runScenario(const ScenarioDef& def,
                 AppConfig&         config,
                 std::ostream&      csv_out,
                 std::ostream&      report_out);
