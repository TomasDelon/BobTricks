#pragma once

/**
 * @file StepPlanner.h
 * @brief Stub de compatibilité pour le planificateur de pas.
 *
 * La logique active de planification des pas réside dans `SimulationCore`
 * (méthodes `stepComputeTriggerState`, `stepFireWalkTrigger`,
 * `stepFireRunTrigger`). Ce fichier reste présent pour maintenir un vocabulaire
 * public cohérent avec la documentation et les scénarios headless existants.
 *
 * @deprecated Utiliser directement `SimulationCore::step()` pour tout nouveau code.
 */
