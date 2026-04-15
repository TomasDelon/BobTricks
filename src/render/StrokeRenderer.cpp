#include "render/StrokeRenderer.h"

#include <cmath>

namespace {

SDL_FPoint add(const SDL_FPoint& a, const SDL_FPoint& b)
{
    return {a.x + b.x, a.y + b.y};
}

SDL_FPoint sub(const SDL_FPoint& a, const SDL_FPoint& b)
{
    return {a.x - b.x, a.y - b.y};
}

SDL_FPoint mul(const SDL_FPoint& v, float s)
{
    return {v.x * s, v.y * s};
}

float length(const SDL_FPoint& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

SDL_FPoint normalizeOrZero(const SDL_FPoint& v)
{
    const float len = length(v);
    if (len < 1.0e-6f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
}

SDL_Vertex makeVertex(const SDL_FPoint& p, SDL_Color color)
{
    SDL_Vertex v{};
    v.position = p;
    v.color = color;
    v.tex_coord = {0.f, 0.f};
    return v;
}

} // namespace

void StrokeRenderer::renderPolyline(SDL_Renderer* renderer,
                                    const std::vector<SDL_FPoint>& points,
                                    float width_px,
                                    SDL_Color color) const
{
    if (!renderer || points.size() < 2 || width_px <= 0.0f) return;

    const float half_w = 0.5f * width_px;
    std::vector<SDL_Vertex> vertices;
    std::vector<int> indices;
    vertices.reserve(points.size() * 2);
    indices.reserve((points.size() - 1) * 6);

    for (std::size_t i = 0; i < points.size(); ++i) {
        SDL_FPoint tangent{};
        if (i == 0) {
            tangent = sub(points[1], points[0]);
        } else if (i + 1 == points.size()) {
            tangent = sub(points[i], points[i - 1]);
        } else {
            tangent = add(sub(points[i], points[i - 1]), sub(points[i + 1], points[i]));
        }
        tangent = normalizeOrZero(tangent);
        const SDL_FPoint normal{-tangent.y, tangent.x};
        const SDL_FPoint offset = mul(normal, half_w);

        vertices.push_back(makeVertex(sub(points[i], offset), color));
        vertices.push_back(makeVertex(add(points[i], offset), color));
    }

    for (std::size_t i = 0; i + 1 < points.size(); ++i) {
        const int base = static_cast<int>(i * 2);
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        indices.push_back(base + 3);
        indices.push_back(base + 2);
    }

    SDL_RenderGeometry(renderer, nullptr,
                       vertices.data(), static_cast<int>(vertices.size()),
                       indices.data(), static_cast<int>(indices.size()));
}
