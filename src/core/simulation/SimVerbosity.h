#pragma once

/**
 * @file SimVerbosity.h
 * @brief Drapeau global de verbosité pour les logs du noyau de simulation.
 */

/**
 * @brief Active la sortie de logs détaillés dans `SimulationCore`.
 *
 * Lorsque ce drapeau est vrai, `SimulationCore::step()` imprime des diagnostics
 * de locomotion sur `stdout`. Il est utilisé uniquement par l'exécutable
 * headless en mode debug et n'est pas thread-safe.
 *
 * @note Il s'agit de la seule variable globale du projet. Elle est justifiée
 * par la nécessité d'activer des logs conditionnels dans le noyau de
 * simulation sans propager une référence de configuration à travers toute
 * la pile d'appels de l'exécutable headless.
 */
extern bool g_sim_verbose;

/**
 * @brief Écrit un message de diagnostic si la verbosité de simulation est active.
 *
 * Le format suit les mêmes règles que `std::fprintf` : le premier argument est
 * une chaîne de format, puis viennent les valeurs à insérer dans cette chaîne.
 */
void simLog(const char* format, ...);
