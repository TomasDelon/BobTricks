#pragma once

#include <vector>
#include "config/AppConfig.h"
#include "core/math/Vec2.h"

/**
 * @brief Terrain procédural stocké sous forme de polyligne triée.
 */
class Terrain
{
public:
    explicit Terrain(const TerrainConfig& config);

    /** @brief Régénère la polyligne à partir de la configuration courante. */
    void generate();

    /** @brief Retourne la hauteur du terrain au point `x`. */
    double height_at(double x) const;

    /** @brief Retourne la pente locale du terrain au point `x`. */
    double slope_at(double x) const;
    /** @brief Retourne la tangente locale unitaire du terrain au point `x`. */
    Vec2   tangent_at(double x) const;
    /** @brief Retourne la normale locale unitaire du terrain au point `x`. */
    Vec2   normal_at(double x) const;

    /** @brief Retourne la polyligne brute utilisée par le renderer de scène. */
    const std::vector<Vec2>& vertices() const { return m_verts; }

private:
    const TerrainConfig& m_config;
    std::vector<Vec2>    m_verts;
};
