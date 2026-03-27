#pragma once

#include "core/math/Vec2.h"

/** @brief Courbe de Bézier quadratique en espace 2D. */
struct BezierQuadratic {
    Vec2 p0;
    Vec2 p1;
    Vec2 p2;

    /** @brief Évalue la courbe pour un paramètre `t` dans `[0,1]`. */
    Vec2 eval(double t) const;
    /** @brief Retourne la tangente non normalisée en `t`. */
    Vec2 tangent(double t) const;
};

/** @brief Courbe de Bézier cubique en espace 2D. */
struct BezierCubic {
    Vec2 p0;
    Vec2 p1;
    Vec2 p2;
    Vec2 p3;

    /** @brief Évalue la courbe pour un paramètre `t` dans `[0,1]`. */
    Vec2 eval(double t) const;
    /** @brief Retourne la tangente non normalisée en `t`. */
    Vec2 tangent(double t) const;
};
