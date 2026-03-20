#include "core/terrain/Terrain.h"

#include <algorithm>
#include <cmath>
#include <random>

static constexpr double WORLD_X_MIN = -300.0;
static constexpr double WORLD_X_MAX =  300.0;
static constexpr double DEG_TO_RAD  = 3.14159265358979323846 / 180.0;

Terrain::Terrain(const TerrainConfig& config)
    : m_config(config)
{}

void Terrain::generate()
{
    m_verts.clear();
    if (!m_config.enabled) return;

    std::mt19937 rng(static_cast<unsigned>(m_config.seed));

    std::uniform_real_distribution<double> len_dist (m_config.seg_min, m_config.seg_max);
    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
    std::uniform_real_distribution<double> small_dist(-m_config.angle_small, m_config.angle_small);
    std::uniform_real_distribution<double> large_dist(-m_config.angle_large, m_config.angle_large);

    double x     = WORLD_X_MIN;
    double y     = 0.0;
    double angle = 0.0;

    m_verts.push_back({x, y});

    while (x < WORLD_X_MAX) {
        const double len   = len_dist(rng);
        const double delta = (prob_dist(rng) < m_config.large_prob)
                           ? large_dist(rng) : small_dist(rng);

        // Soft height bounds — nudge angle back toward horizontal
        double bias = 0.0;
        if (y > m_config.height_max) bias = -20.0;
        if (y < m_config.height_min) bias =  20.0;

        angle = std::clamp(angle + delta + bias,
                           -m_config.slope_max, m_config.slope_max);

        const double rad = angle * DEG_TO_RAD;
        x += std::cos(rad) * len;
        y += std::sin(rad) * len;

        m_verts.push_back({x, y});
    }

    // Sentinel point just beyond the right edge
    m_verts.push_back({WORLD_X_MAX + 10.0, m_verts.back().y});
}

double Terrain::height_at(double x) const
{
    if (!m_config.enabled || m_verts.size() < 2) return 0.0;

    // Binary search for the first vertex with v.x >= x
    const auto it = std::lower_bound(
        m_verts.begin(), m_verts.end(), x,
        [](const Vec2& v, double val) { return v.x < val; });

    if (it == m_verts.end())   return m_verts.back().y;
    if (it == m_verts.begin()) return m_verts.front().y;

    const Vec2& b = *it;
    const Vec2& a = *std::prev(it);
    const double t = (x - a.x) / (b.x - a.x);
    return a.y + t * (b.y - a.y);
}
