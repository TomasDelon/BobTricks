#pragma once

#include <functional>
#include <map>
#include <string>

#include "config/AppConfig.h"
#include "headless/ScenarioDef.h"

// A factory that builds a ScenarioDef from the loaded AppConfig.
// The AppConfig is passed so scenarios can compute nominal heights, etc.
using ScenarioFactory = std::function<ScenarioDef(const AppConfig&)>;

// Returns the catalog of all registered scenarios (name → factory).
// Add new scenarios to ScenarioLibrary.cpp — no other file needs to change.
const std::map<std::string, ScenarioFactory>& scenarioLibrary();
