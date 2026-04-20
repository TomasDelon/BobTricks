#pragma once

/**
 * @file Bezier.h
 * @brief Courbes de Bézier quadratiques et cubiques utilisées par le renderer spline.
 */

#include "core/math/Vec2.h"

/**
 * @brief Courbe de Bézier quadratique (degré 2) en espace 2D.
 *
 * Définie par un point de départ `p0`, un point de contrôle `p1` et un point
 * d'arrivée `p2`. Utilisée pour les arcs de tête et certaines transitions douces.
 */
struct BezierQuadratic {
    Vec2 p0; ///< Point de départ.
    Vec2 p1; ///< Point de contrôle.
    Vec2 p2; ///< Point d'arrivée.

    /**
     * @brief Évalue la courbe pour un paramètre `t` dans `[0,1]`.
     * @param t Paramètre de la courbe (0 = début, 1 = fin).
     * @return Position sur la courbe.
     */
    Vec2 eval(double t) const;
    /**
     * @brief Retourne la tangente non normalisée en `t`.
     * @param t Paramètre de la courbe.
     * @return Vecteur tangent (non unitaire).
     */
    Vec2 tangent(double t) const;
};

/**
 * @brief Courbe de Bézier cubique (degré 3) en espace 2D.
 *
 * Définie par deux points extrêmes (`p0`, `p3`) et deux points de contrôle
 * internes (`p1`, `p2`). Utilisée pour les membres du corps dans le renderer
 * spline (jambes, bras, torse).
 */
struct BezierCubic {
    Vec2 p0; ///< Point de départ.
    Vec2 p1; ///< Premier point de contrôle.
    Vec2 p2; ///< Second point de contrôle.
    Vec2 p3; ///< Point d'arrivée.

    /**
     * @brief Évalue la courbe pour un paramètre `t` dans `[0,1]`.
     * @param t Paramètre de la courbe (0 = début, 1 = fin).
     * @return Position sur la courbe.
     */
    Vec2 eval(double t) const;
    /**
     * @brief Retourne la tangente non normalisée en `t`.
     * @param t Paramètre de la courbe.
     * @return Vecteur tangent (non unitaire).
     */
    Vec2 tangent(double t) const;
};
