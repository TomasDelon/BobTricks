#pragma once

/**
 * @file DustParticle.h
 * @brief Particule de poussière émise lors des contacts pied-sol.
 */

#include "core/math/Vec2.h"

#include <cstdint>

/**
 * @brief Données d'une particule de poussière gérée par `EffectsSystem`.
 *
 * Les particules sont stockées dans un `std::deque` et rendues par
 * `SceneRenderer::drawDust()`. Leur opacité s'estompe linéairement de `alpha`
 * à zéro au cours de leur durée de vie `lifetime_s`.
 */
struct DustParticle {
    double spawn_time = 0.0; ///< Temps de simulation à la création (s).
    double lifetime_s = 0.0; ///< Durée de vie totale de la particule (s).
    Vec2   pos        = {0.0, 0.0}; ///< Position monde courante (m).
    Vec2   vel        = {0.0, 0.0}; ///< Vitesse monde courante (m/s).
    float  radius_px  = 1.0f;       ///< Rayon à l'écran (px).
    float  alpha      = 0.0f;       ///< Opacité initiale `[0, 255]`.
    float  stretch    = 1.0f;       ///< Facteur d'étirement selon la vitesse.
    std::uint8_t color_r = 214; ///< Canal rouge de la couleur de base.
    std::uint8_t color_g = 198; ///< Canal vert de la couleur de base.
    std::uint8_t color_b = 170; ///< Canal bleu de la couleur de base.
};
