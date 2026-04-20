#pragma once

/**
 * @file Terrain.h
 * @brief Terrain procédural 2D généré par la méthode angle-walk.
 */

#include <vector>
#include "config/AppConfig.h"
#include "core/math/Vec2.h"

/**
 * @brief Terrain procédural 2D stocké sous forme de polyligne triée par X.
 *
 * La polyligne est toujours triée par coordonnée X croissante. Toutes les
 * requêtes `height_at`, `slope_at`, `tangent_at` et `normal_at` effectuent
 * une recherche dichotomique suivie d'une interpolation linéaire entre les
 * deux sommets encadrants. En dehors des extrémités, le terrain est extrapolé
 * horizontalement (hauteur constante).
 */
class Terrain
{
public:
    /**
     * @brief Construit le terrain à partir de la configuration donnée.
     * @param config Référence non possédée sur `AppConfig::terrain`.
     */
    explicit Terrain(const TerrainConfig& config);

    /**
     * @brief Régénère la polyligne à partir de la configuration courante.
     *
     * Doit être appelée après toute modification de `TerrainConfig` (graine,
     * paramètres géométriques, flag `enabled`).
     */
    void generate();

    /**
     * @brief Retourne la hauteur interpolée du terrain au point `x`.
     * @param x Coordonnée X monde (m).
     * @return Hauteur Y (m).
     */
    double height_at(double x) const;

    /**
     * @brief Retourne la pente locale `dy/dx` au point `x`.
     * @param x Coordonnée X monde (m).
     * @return Pente (sans unité ; 0 = plat, >0 = montée).
     */
    double slope_at(double x) const;

    /**
     * @brief Retourne la tangente locale unitaire au point `x`.
     * @param x Coordonnée X monde (m).
     * @return Vecteur unitaire dans le sens croissant de X.
     */
    Vec2   tangent_at(double x) const;

    /**
     * @brief Retourne la normale locale unitaire au point `x`.
     * @param x Coordonnée X monde (m).
     * @return Vecteur unitaire dirigé vers le haut (extérieur du terrain).
     */
    Vec2   normal_at(double x) const;

    /** @brief Retourne la polyligne brute utilisée par `SceneRenderer`. */
    const std::vector<Vec2>& vertices() const { return m_verts; }

private:
    const TerrainConfig& m_config;
    std::vector<Vec2>    m_verts;
};
