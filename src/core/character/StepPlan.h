#pragma once

/**
 * @file StepPlan.h
 * @brief Plan de pas élémentaire conservé pour la compatibilité avec d'anciens tests.
 *
 * @note Ce type est un vestige de l'architecture initiale.
 * La logique active de planification des pas réside désormais entièrement dans
 * `SimulationCore` via `FootState` et `StepCtx`.
 */

#include "core/math/Vec2.h"

/**
 * @brief Plan de pas élémentaire utilisé par les anciennes interfaces de marche.
 *
 * Conservé pour la compatibilité ascendante avec certaines assertions de
 * scénarios headless. Ne pas utiliser dans les nouvelles implémentations.
 */
struct StepPlan {
    bool   active       = false;       ///< Vrai si un pas est en cours.
    bool   move_right   = false;       ///< Vrai si le pied droit est en vol.
    Vec2   takeoff_pos  = {0.0, 0.0};  ///< Position de départ du pied au décollage.
    Vec2   land_target  = {0.0, 0.0};  ///< Position cible d'atterrissage.
    double swing_t      = 0.0;         ///< Progression du swing dans `[0, 1]`, avance à `step_speed × dt`.
};
