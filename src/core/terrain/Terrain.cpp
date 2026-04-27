#include "core/terrain/Terrain.h"
#include "core/math/MathConstants.h"

#include <algorithm>
#include <cmath>
#include <random>

static constexpr double WORLD_X_MIN = -300.0;
static constexpr double WORLD_X_MAX =  300.0;

namespace {

struct TerrainSegment {
    Vec2 a;
    Vec2 b;
};

bool pointXLessThan(const Vec2& point, double x)
{
    return point.x < x;
}

TerrainSegment findSegment(const std::vector<Vec2>& verts, double x)
{
    if (verts.size() < 2) return {};

    const std::vector<Vec2>::const_iterator it = std::lower_bound(
        verts.begin(), verts.end(), x, pointXLessThan);

    if (it == verts.end()) {
        return {verts[verts.size() - 2], verts.back()};
    }
    if (it == verts.begin()) {
        return {verts.front(), verts[1]};
    }
    return {*std::prev(it), *it};
}

Vec2 normalizeOrFallback(const Vec2& v, const Vec2& fallback)
{
    const double len = v.length();
    return (len > 1e-9) ? (v / len) : fallback;
}

}  // fin namespace

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

        // Bornes de hauteur souples : ramener légèrement l'angle vers l'horizontale.
        double bias = 0.0;
        if (y > m_config.height_max) bias = -20.0;
        if (y < m_config.height_min) bias =  20.0;

        angle = std::clamp(angle + delta + bias,
                           -m_config.slope_max, m_config.slope_max);

        const double rad = angle * kDegToRad;
        x += std::cos(rad) * len;
        y += std::sin(rad) * len;

        m_verts.push_back({x, y});
    }

    // Point sentinelle juste au-delà du bord droit.
    m_verts.push_back({WORLD_X_MAX + 10.0, m_verts.back().y});
}

double Terrain::height_at(double x) const
{
    if (!m_config.enabled || m_verts.size() < 2) return 0.0;

    const TerrainSegment seg = findSegment(m_verts, x);
    const Vec2& a = seg.a;
    const Vec2& b = seg.b;
    const double t = (x - a.x) / (b.x - a.x);
    return a.y + t * (b.y - a.y);
}

double Terrain::slope_at(double x) const
{
    if (!m_config.enabled || m_verts.size() < 2) return 0.0;

    const TerrainSegment seg = findSegment(m_verts, x);
    const double dx = seg.b.x - seg.a.x;
    if (std::abs(dx) <= 1e-9) return 0.0;
    return (seg.b.y - seg.a.y) / dx;
}

Vec2 Terrain::tangent_at(double x) const
{
    if (!m_config.enabled || m_verts.size() < 2) return {1.0, 0.0};

    const TerrainSegment seg = findSegment(m_verts, x);
    return normalizeOrFallback(seg.b - seg.a, {1.0, 0.0});
}

Vec2 Terrain::normal_at(double x) const
{
    const Vec2 t = tangent_at(x);
    return {-t.y, t.x};
}

const std::vector<Vec2>& Terrain::vertices() const
{
    return m_verts;
}
