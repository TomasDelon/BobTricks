#pragma once

/**
 * @file ScenarioLibrary.h
 * @brief Catalogue statique de scénarios headless prédéfinis.
 */

#include <functional>
#include <map>
#include <string>

#include "config/AppConfig.h"
#include "headless/ScenarioDef.h"

/**
 * @brief Fabrique de scénario paramétrée par la configuration de l'application.
 *
 * Chaque entrée du catalogue est une fonction qui construit un `ScenarioDef`
 * à partir d'une `AppConfig` chargée. Cela permet à un scénario de s'adapter
 * aux paramètres courants (terrain, physique, etc.).
 */
using ScenarioFactory = std::function<ScenarioDef(const AppConfig&)>;

/**
 * @brief Retourne le catalogue statique des scénarios enregistrés.
 *
 * Chaque clé est le nom du scénario tel qu'il apparaît en ligne de commande.
 * Les scénarios disponibles comprennent au minimum : marche plate, pente,
 * arrêt d'urgence et saut.
 *
 * @return Référence constante sur le catalogue (durée de vie statique).
 */
const std::map<std::string, ScenarioFactory>& scenarioLibrary();
