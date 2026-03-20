#pragma once

#include <vector>
#include "config/AppConfig.h"
#include "core/math/Vec2.h"

// Procedural terrain stored as a sorted polyline.
// height_at(x) returns height above GROUND_Y via O(log n) binary search.
// Call generate() once after config is loaded, and again to regenerate.
class Terrain
{
public:
    explicit Terrain(const TerrainConfig& config);

    // (Re)build the polyline from the current config.
    void generate();

    // Height above GROUND_Y at world-x. Returns 0 when disabled or out of range.
    double height_at(double x) const;

    // Raw polyline — used by SceneRenderer.
    const std::vector<Vec2>& vertices() const { return m_verts; }

private:
    const TerrainConfig& m_config;
    std::vector<Vec2>    m_verts;
};
