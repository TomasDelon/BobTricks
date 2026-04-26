#pragma once

#include <vector>

#include "core/math/Bezier.h"

/**
 * @file StrokePath.h
 * @brief Chemin vectoriel 2D composé de segments droits et de courbes de Bézier.
 */

/**
 * @brief Description d'un chemin vectoriel composé de segments et de Béziers.
 *
 * Le chemin ne dessine rien par lui-même : il accumule des commandes de tracé
 * (`moveTo`, `lineTo`, `quadTo`, `cubicTo`, `closePath`) puis les résout en
 * polyligne via `flatten()`. Cette polyligne est ensuite rendue par
 * `StrokeRenderer::renderPolyline()`.
 *
 * @par Exemple d'utilisation
 * @code
 * StrokePath path;
 * path.moveTo(shoulder);
 * path.cubicTo(c1, c2, hand);
 * std::vector<Vec2> pts = path.flatten(24);
 * // convertir pts en SDL_FPoint puis appeler StrokeRenderer
 * @endcode
 */
class StrokePath
{
public:
    /** @brief Supprime toutes les commandes du chemin. */
    void clear();
    /** @brief Démarre un sous-chemin à une position donnée. */
    void moveTo(const Vec2& p);
    /** @brief Ajoute un segment droit jusqu'à `p`. */
    void lineTo(const Vec2& p);
    /** @brief Ajoute une Bézier quadratique. */
    void quadTo(const Vec2& c, const Vec2& p);
    /** @brief Ajoute une Bézier cubique. */
    void cubicTo(const Vec2& c1, const Vec2& c2, const Vec2& p);
    /** @brief Referme le sous-chemin courant. */
    void closePath();

    /** @brief Échantillonne le chemin en polyligne. */
    std::vector<Vec2> flatten(int samples_per_curve) const;
    /** @brief Retourne les points de contrôle du chemin pour le debug. */
    std::vector<Vec2> controlPolygon() const;

private:
    enum class CmdType {
        MoveTo,
        LineTo,
        QuadTo,
        CubicTo,
        ClosePath
    };

    struct Command {
        CmdType type;
        Vec2 a;
        Vec2 b;
        Vec2 c;
    };

    std::vector<Command> m_commands;
};
