#pragma once

#include <vector>

#include "core/math/Bezier.h"

/**
 * @brief Description d'un chemin vectoriel composé de segments et de Béziers.
 *
 * Le chemin ne dessine rien par lui-même ; il sert de représentation
 * intermédiaire avant l'échantillonnage puis le rendu par `StrokeRenderer`.
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
