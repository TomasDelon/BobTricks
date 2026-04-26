#include "render/SceneRenderer.h"

#include <algorithm>
#include <cmath>
#include <vector>

static constexpr double GRID_SPACING_M         = 1.0;
static constexpr float  GROUND_THICKNESS_PX    = 4.0f;

namespace {

void drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius)
{
    const int ir = static_cast<int>(std::ceil(radius));
    for (int dy = -ir; dy <= ir; ++dy) {
        const float fy = static_cast<float>(dy);
        const float dx = std::sqrt(std::max(0.f, radius * radius - fy * fy));
        SDL_RenderDrawLineF(renderer, cx - dx, cy + fy, cx + dx, cy + fy);
    }
}

void drawMotionStreak(SDL_Renderer* renderer,
                      float x0,
                      float y0,
                      float x1,
                      float y1,
                      float radius)
{
    const int layers = std::max(1, static_cast<int>(std::ceil(radius * 0.45f)));
    for (int i = -layers; i <= layers; ++i) {
        const float off = static_cast<float>(i);
        SDL_RenderDrawLineF(renderer, x0, y0 + off, x1, y1 + off);
    }
}

void addWorldPoint(std::vector<SDL_FPoint>& pts,
                   const Camera2D& camera,
                   double wx,
                   double wy,
                   double ground_y,
                   int viewport_w,
                   int viewport_h)
{
    pts.push_back(camera.worldToScreen(wx, wy, ground_y, viewport_w, viewport_h));
}

bool pointXLessThan(const Vec2& point, double x)
{
    return point.x < x;
}

} // namespace

void SceneRenderer::drawGrid(SDL_Renderer* renderer,
                             const Camera2D& camera,
                             double ground_y,
                             int viewport_w, int viewport_h) const
{
    const Vec2   center           = camera.getCenter();
    const double ppm              = camera.getPixelsPerMeter();
    const double ground_margin    = camera.getGroundMarginPx();
    const double half_w_m         = viewport_w / ppm * 0.5;
    const double world_below      = ground_margin / ppm;
    const double world_above      = (viewport_h - ground_margin) / ppm;

    const double cy = ground_y + center.y;  // world-Y at camera vertical center

    const int x_min = static_cast<int>(std::floor(center.x - half_w_m)) - 1;
    const int x_max = static_cast<int>(std::ceil(center.x  + half_w_m)) + 1;
    const int y_min = static_cast<int>(std::floor(cy - world_below))     - 1;
    const int y_max = static_cast<int>(std::ceil(cy  + world_above))     + 1;

    SDL_SetRenderDrawColor(renderer, 36, 36, 36, 255);

    for (int gx = x_min; gx <= x_max; ++gx) {
        const double wx = gx * GRID_SPACING_M;
        const SDL_FPoint p0 = camera.worldToScreen(wx, static_cast<double>(y_min), ground_y, viewport_w, viewport_h);
        const SDL_FPoint p1 = camera.worldToScreen(wx, static_cast<double>(y_max), ground_y, viewport_w, viewport_h);
        SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
    }

    for (int gy = y_min; gy <= y_max; ++gy) {
        const double wy = gy * GRID_SPACING_M;
        const SDL_FPoint p0 = camera.worldToScreen(static_cast<double>(x_min), wy, ground_y, viewport_w, viewport_h);
        const SDL_FPoint p1 = camera.worldToScreen(static_cast<double>(x_max), wy, ground_y, viewport_w, viewport_h);
        SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
    }
}

void SceneRenderer::drawGround(SDL_Renderer* renderer,
                               const Camera2D& camera,
                               const Terrain& terrain,
                               double ground_y,
                               int viewport_w, int viewport_h) const
{
    const Vec2   center = camera.getCenter();
    const double ppm    = camera.getPixelsPerMeter();
    const double half_w = viewport_w / ppm * 0.5;

    const double x_left  = center.x - half_w - 2.0;
    const double x_right = center.x + half_w + 2.0;

    // ── Collect visible screen points ────────────────────────────────────────
    // Use terrain vertices directly (O(log n) to find start, O(K) to iterate).
    // Add virtual endpoints at the viewport edges for a clean fill.
    std::vector<SDL_FPoint> pts;

    const std::vector<Vec2>& verts = terrain.vertices();

    if (verts.size() < 2) {
        // Terrain disabled or not generated — flat ground
        addWorldPoint(pts, camera, x_left,  ground_y, ground_y, viewport_w, viewport_h);
        addWorldPoint(pts, camera, x_right, ground_y, ground_y, viewport_w, viewport_h);
    } else {
        // Virtual left edge (always first, x strictly increasing from here)
        addWorldPoint(pts, camera, x_left, ground_y + terrain.height_at(x_left),
                      ground_y, viewport_w, viewport_h);

        // All vertices strictly inside the visible range (left to right, no step-back)
        std::vector<Vec2>::const_iterator it = std::lower_bound(
            verts.begin(), verts.end(), x_left, pointXLessThan);
        for (; it != verts.end() && it->x < x_right; ++it)
            addWorldPoint(pts, camera, it->x, ground_y + it->y,
                          ground_y, viewport_w, viewport_h);

        // Virtual right edge
        addWorldPoint(pts, camera, x_right, ground_y + terrain.height_at(x_right),
                      ground_y, viewport_w, viewport_h);
    }

    // ── Fill: multi-pass scanline gradient (no SDL_RenderGeometry artifacts) ─
    // Each pass covers a fraction of the fill height with its own alpha.
    // Passes accumulate: full opacity near surface, fades toward bottom.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    {
        const double gm    = camera.getGroundMarginPx();
        const float  bot_y = static_cast<float>(viewport_h);

        // Precompute terrain screen-y per column (one height_at call each)
        std::vector<float> tsys(static_cast<std::size_t>(viewport_w));
        for (int sx = 0; sx < viewport_w; ++sx) {
            const double wx = center.x + (sx - viewport_w * 0.5) / ppm;
            tsys[static_cast<std::size_t>(sx)] = static_cast<float>(
                viewport_h - gm - (terrain.height_at(wx) - center.y) * ppm);
        }

        // Each band covers fill_frac of the column; bands accumulate (additive alpha).
        // 16 bands, equal alpha → smooth gradient: ~160 alpha at surface, 10 at bottom.
        struct Band { float frac; Uint8 a; };
        static constexpr Band kBands[] = {
            {1.00f, 10},
            {0.93f, 10},
            {0.86f, 10},
            {0.79f, 10},
            {0.72f, 10},
            {0.65f, 10},
            {0.58f, 10},
            {0.51f, 10},
            {0.44f, 10},
            {0.37f, 10},
            {0.30f, 10},
            {0.23f, 10},
            {0.17f, 10},
            {0.11f, 10},
            {0.06f, 10},
            {0.02f, 10},
        };

        for (const Band& band : kBands) {
            SDL_SetRenderDrawColor(renderer, 90, 90, 90, band.a);
            for (int sx = 0; sx < viewport_w; ++sx) {
                const float tsy    = tsys[static_cast<std::size_t>(sx)];
                const float fill_h = bot_y - tsy;
                if (fill_h <= 0.f) continue;
                SDL_RenderDrawLineF(renderer,
                                    static_cast<float>(sx), tsy,
                                    static_cast<float>(sx), tsy + fill_h * band.frac);
            }
        }
    }

    // ── Terrain surface outline — drawn over the fill ────────────────────────
    SDL_SetRenderDrawColor(renderer, 90, 90, 90, 255);
    const int half = static_cast<int>(GROUND_THICKNESS_PX * 0.5f);
    for (int off = -half; off <= half; ++off)
        for (std::size_t i = 0; i + 1 < pts.size(); ++i)
            SDL_RenderDrawLineF(renderer,
                                pts[i].x,     pts[i].y     + static_cast<float>(off),
                                pts[i+1].x,   pts[i+1].y   + static_cast<float>(off));
}

void SceneRenderer::drawDust(SDL_Renderer* renderer,
                             const Camera2D& camera,
                             const std::deque<DustParticle>& dustParticles,
                             double sim_time,
                             double ground_y,
                             int viewport_w,
                             int viewport_h) const
{
    if (dustParticles.empty()) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (const DustParticle& particle : dustParticles) {
        if (particle.lifetime_s <= 0.0) continue;
        const double age = sim_time - particle.spawn_time;
        if (age < 0.0 || age >= particle.lifetime_s) continue;

        const double life = age / particle.lifetime_s;
        const Vec2 world_pos = particle.pos + particle.vel * age;
        const SDL_FPoint ps = camera.worldToScreen(world_pos.x, world_pos.y, ground_y, viewport_w, viewport_h);
        const float radius = std::max(0.75f, particle.radius_px * static_cast<float>(1.0 - 0.30 * life));
        const float alpha_f = std::clamp(particle.alpha * static_cast<float>(1.0 - life), 0.0f, 255.0f);
        const Uint8 alpha = static_cast<Uint8>(alpha_f);
        const Vec2 streak_world = particle.vel * (0.024 * particle.stretch * (1.0 - 0.45 * life));
        const SDL_FPoint tail = camera.worldToScreen(world_pos.x - streak_world.x,
                                                     world_pos.y - streak_world.y,
                                                     ground_y, viewport_w, viewport_h);

        SDL_SetRenderDrawColor(renderer, particle.color_r, particle.color_g, particle.color_b,
                               static_cast<Uint8>(alpha_f * 0.24f));
        drawMotionStreak(renderer, tail.x, tail.y, ps.x, ps.y, radius);

        SDL_SetRenderDrawColor(renderer, particle.color_r, particle.color_g, particle.color_b,
                               static_cast<Uint8>(alpha_f * 0.38f));
        drawFilledCircle(renderer, ps.x, ps.y, radius * (1.55f + 0.12f * particle.stretch));

        SDL_SetRenderDrawColor(renderer, particle.color_r, particle.color_g, particle.color_b, alpha);
        drawFilledCircle(renderer, ps.x, ps.y, radius);

        SDL_SetRenderDrawColor(renderer, 248, 240, 226, static_cast<Uint8>(alpha_f * 0.48f));
        drawFilledCircle(renderer, ps.x, ps.y, radius * 0.42f);
    }
}

void SceneRenderer::render(SDL_Renderer* renderer,
                           const Camera2D& camera,
                           const Terrain& terrain,
                           const std::deque<DustParticle>& dustParticles,
                           bool show_background_grid,
                           double sim_time,
                           double ground_y,
                           int viewport_w, int viewport_h) const
{
    if (show_background_grid)
        drawGrid(renderer, camera, ground_y, viewport_w, viewport_h);
    drawGround(renderer, camera, terrain, ground_y, viewport_w, viewport_h);
    drawDust(renderer, camera, dustParticles, sim_time, ground_y, viewport_w, viewport_h);
}
