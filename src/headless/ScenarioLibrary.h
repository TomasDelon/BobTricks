#pragma once

#include <functional>
#include <map>
#include <string>

#include "config/AppConfig.h"
#include "headless/ScenarioDef.h"

/** @brief Fabrique de scénario à partir de la configuration chargée. */
using ScenarioFactory = std::function<ScenarioDef(const AppConfig&)>;

/** @brief Retourne le catalogue des scénarios enregistrés. */
const std::map<std::string, ScenarioFactory>& scenarioLibrary();
