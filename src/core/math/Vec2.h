#pragma once

/**
 * @file Vec2.h
 * @brief Vecteur 2D léger utilisé dans tout le noyau de simulation.
 */

#include <cmath>
#include "core/math/MathConstants.h"

/**
 * @brief Vecteur 2D à double précision.
 *
 * Cette structure couvre tous les besoins vectoriels du noyau : positions,
 * vitesses, normales de terrain et points de contrôle des splines.
 */
struct Vec2 {
    double x = 0.0; ///< Composante horizontale.
    double y = 0.0; ///< Composante verticale.

    /** @brief Addition composante par composante. */
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    /** @brief Soustraction composante par composante. */
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    /** @brief Multiplication par un scalaire. */
    Vec2 operator*(double s)      const { return {x * s,   y * s};   }
    /** @brief Division par un scalaire (non nul). */
    Vec2 operator/(double s)      const { return {x / s,   y / s};   }

    /** @brief Addition en place. */
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    /** @brief Soustraction en place. */
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }

    /** @brief Norme euclidienne du vecteur. */
    double length() const { return std::sqrt(x * x + y * y); }
};

/**
 * @brief Produit scalaire de deux vecteurs.
 * @return `a.x*b.x + a.y*b.y`
 */
inline double dot(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

/**
 * @brief Normalise `v` ou retourne `fallback` si `v` est quasi-nul.
 *
 * Évite les divisions par zéro dans les calculs de direction en terrain plat
 * ou quand deux points coïncident.
 *
 * @param v        Vecteur à normaliser.
 * @param fallback Vecteur retourné si `||v|| <= kEpsLength`.
 * @return `v / ||v||` ou `fallback`.
 */
inline Vec2 normalizeOr(Vec2 v, Vec2 fallback)
{
    const double len = v.length();
    if (len <= kEpsLength) return fallback;
    return v / len;
}
