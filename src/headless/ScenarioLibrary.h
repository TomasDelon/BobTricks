#pragma once

#include <functional>
#include <map>
#include <string>

#include "config/AppConfig.h"
#include "headless/ScenarioDef.h"

/**
 * @brief Fabrique de scénario à partir de la configuration chargée.
 *
 * Chaque fabrique produit une définition autonome utilisable aussi bien par les
 * tests de régression que par l'exécutable headless.
 */
using ScenarioFactory = std::function<ScenarioDef(const AppConfig&)>;

/**
 * @brief Retourne le catalogue des scénarios enregistrés.
 * @return Mapping immuable `nom -> fabrique`.
 */
const std::map<std::string, ScenarioFactory>& scenarioLibrary();
