#pragma once

/**
 * @file WorldConstants.h
 * @brief Constantes monde partagées entre le noyau et les exécutables.
 *
 * Ces constantes définissent le repère de référence monde de BobTricks.
 * L'axe Y est dirigé vers le haut ; le niveau du sol est `GROUND_Y = 0`.
 */

namespace World {

/**
 * @brief Niveau Y de base du terrain en coordonnées monde (m).
 *
 * Le terrain procédural est généré autour de ce niveau de référence.
 * La conversion monde→écran dans `Camera2D` utilise ce paramètre
 * pour positionner la ligne de sol dans la fenêtre SDL.
 */
inline constexpr double GROUND_Y = 0.0;

} // namespace World
