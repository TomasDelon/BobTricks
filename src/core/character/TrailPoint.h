#pragma once

/**
 * @file TrailPoint.h
 * @brief Échantillon horodaté de la trajectoire du centre de masse.
 */

#include "core/math/Vec2.h"

/**
 * @brief Point horodaté stocké dans le buffer de traînée du CM.
 *
 * La traînée est rendue dans `DebugOverlayRenderer` pour visualiser la
 * trajectoire récente du centre de masse. Les points plus anciens que
 * `CMConfig::trail_duration` secondes sont élagués à chaque frame.
 */
struct TrailPoint {
    double time; ///< Temps de simulation lors de la capture (s).
    Vec2   pos;  ///< Position monde du CM à cet instant (m).
};
