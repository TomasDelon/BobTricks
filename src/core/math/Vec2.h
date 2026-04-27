#pragma once

/**
 * @file Vec2.h
 * @brief Vecteur 2D léger utilisé dans tout le noyau de simulation.
 */

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
    Vec2 operator+(const Vec2& o) const;
    /** @brief Soustraction composante par composante. */
    Vec2 operator-(const Vec2& o) const;
    /** @brief Multiplication par un scalaire. */
    Vec2 operator*(double s) const;
    /** @brief Division par un scalaire (non nul). */
    Vec2 operator/(double s) const;

    /** @brief Addition en place. */
    Vec2& operator+=(const Vec2& o);
    /** @brief Soustraction en place. */
    Vec2& operator-=(const Vec2& o);

    /** @brief Norme euclidienne du vecteur. */
    double length() const;
};

/**
 * @brief Produit scalaire de deux vecteurs.
 * @return `a.x*b.x + a.y*b.y`
 */
double dot(const Vec2& a, const Vec2& b);

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
Vec2 normalizeOr(const Vec2& v, const Vec2& fallback);
