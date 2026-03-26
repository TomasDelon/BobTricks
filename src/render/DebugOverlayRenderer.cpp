#include "render/DebugOverlayRenderer.h"
#include "core/character/BalanceState.h"
#include "core/physics/Geometry.h"

#include <cmath>
#include <algorithm>

void DebugOverlayRenderer::drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius)
{
    const int ir = static_cast<int>(std::ceil(radius));
    for (int dy = -ir; dy <= ir; ++dy) {
        const float fy = static_cast<float>(dy);
        const float dx = std::sqrt(std::max(0.f, radius * radius - fy * fy));
        SDL_RenderDrawLineF(renderer, cx - dx, cy + fy, cx + dx, cy + fy);
    }
}

void DebugOverlayRenderer::drawArrowHead(SDL_Renderer* renderer,
                                          float fx, float fy, float tx, float ty, float size)
{
    const float dx  = tx - fx, dy  = ty - fy;
    const float len = std::sqrt(dx*dx + dy*dy);
    if (len < 1.f) return;
    const float ux = dx/len, uy = dy/len;
    const float px = -uy,    py =  ux;
    SDL_RenderDrawLineF(renderer, tx, ty, tx - ux*size + px*size*0.5f, ty - uy*size + py*size*0.5f);
    SDL_RenderDrawLineF(renderer, tx, ty, tx - ux*size - px*size*0.5f, ty - uy*size - py*size*0.5f);
}

void DebugOverlayRenderer::drawComponentArrow(SDL_Renderer* renderer,
                                               float fx, float fy, float tx, float ty,
                                               Uint8 r, Uint8 g, Uint8 b)
{
    const float dx = tx - fx, dy = ty - fy;
    if (dx*dx + dy*dy < 4.f) return;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawLineF(renderer, fx, fy, tx, ty);
    drawArrowHead(renderer, fx, fy, tx, ty, 8.f);
}

// ─────────────────────────────────────────────────────────────────────────────

void DebugOverlayRenderer::drawSupportInterval(SDL_Renderer*       renderer,
                                                const Camera2D&     camera,
                                                const SupportState& support,
                                                double              ground_y,
                                                int                 vw,
                                                int                 vh)
{
    if (!support.left_planted && !support.right_planted) return;  // airborne

    SDL_SetRenderDrawColor(renderer, 255, 160, 30, 220);

    if (support.both_planted()) {
        // Double support: segment connecting the two actual foot contacts.
        const SDL_FPoint pl = camera.worldToScreen(support.x_left,  support.y_left,  ground_y, vw, vh);
        const SDL_FPoint pr = camera.worldToScreen(support.x_right, support.y_right, ground_y, vw, vh);
        SDL_RenderDrawLineF(renderer, pl.x, pl.y,     pr.x, pr.y);
        SDL_RenderDrawLineF(renderer, pl.x, pl.y + 1, pr.x, pr.y + 1);
    } else {
        // Single support: small cross at the stance foot.
        const double stance_y = support.left_planted ? support.y_left : support.y_right;
        const SDL_FPoint p = camera.worldToScreen(support.center(), stance_y, ground_y, vw, vh);
        SDL_RenderDrawLineF(renderer, p.x - 4.f, p.y, p.x + 4.f, p.y);
        SDL_RenderDrawLineF(renderer, p.x,        p.y - 5.f, p.x, p.y + 5.f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void DebugOverlayRenderer::renderBackground(SDL_Renderer*                 renderer,
                                             const Camera2D&               camera,
                                             const CMState&                cm,
                                             const Terrain&                terrain,
                                             const CharacterConfig&        charConfig,
                                             const CMConfig&               cmConfig,
                                             const std::deque<TrailPoint>& trail,
                                             double                        sim_time,
                                             double                        ground_y,
                                             int                           viewport_w,
                                             int                           viewport_h,
                                             const SupportState&           support) const
{
    // CM trail
    if (cmConfig.show_trail && trail.size() >= 2) {
        const double inv_dur = 1.0 / cmConfig.trail_duration;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (std::size_t i = 0; i + 1 < trail.size(); ++i) {
            const double age = sim_time - trail[i].time;
            const Uint8  a   = static_cast<Uint8>((1.0 - age * inv_dur) * 220.0);
            const SDL_FPoint p0 = camera.worldToScreen(trail[i].pos.x,   trail[i].pos.y,   ground_y, viewport_w, viewport_h);
            const SDL_FPoint p1 = camera.worldToScreen(trail[i+1].pos.x, trail[i+1].pos.y, ground_y, viewport_w, viewport_h);
            SDL_SetRenderDrawColor(renderer, 0, 170, 255, a);
            SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
        }
    }

    // Smoothed ground reference: sample points ± L and segment between them (green)
    if (cmConfig.show_ground_reference) {
        const double L   = charConfig.body_height_m / 5.0;
        const double hb  = ground_y + terrain.height_at(cm.position.x - L);
        const double hf  = ground_y + terrain.height_at(cm.position.x + L);
        const SDL_FPoint pb = camera.worldToScreen(cm.position.x - L, hb, ground_y, viewport_w, viewport_h);
        const SDL_FPoint pf = camera.worldToScreen(cm.position.x + L, hf, ground_y, viewport_w, viewport_h);
        SDL_SetRenderDrawColor(renderer, 50, 220, 50, 255);
        SDL_RenderDrawLineF(renderer, pb.x, pb.y, pf.x, pf.y);
        drawFilledCircle(renderer, pb.x, pb.y, 3.f);
        drawFilledCircle(renderer, pf.x, pf.y, 3.f);
    }

    // Support interval — segment between planted feet at ground level
    if (cmConfig.show_support_line)
        drawSupportInterval(renderer, camera, support, ground_y, viewport_w, viewport_h);
}

// ─────────────────────────────────────────────────────────────────────────────

void DebugOverlayRenderer::renderForeground(SDL_Renderer*          renderer,
                                             const Camera2D&        camera,
                                             const CMState&         cm,
                                             const CharacterConfig& charConfig,
                                             const StandingConfig&  standConfig,
                                             const CMConfig&        cmConfig,
                                             const BalanceState&    balance,
                                             const StepPlan&        step_plan,
                                             bool                   feet_initialized,
                                             double                 ref_h,
                                             double                 accel_display_scale,
                                             bool                   drag_active,
                                             float                  drag_mouse_x,
                                             float                  drag_mouse_y,
                                             double                 ground_y,
                                             int                    viewport_w,
                                             int                    viewport_h) const
{
    const SDL_FPoint cm_s     = camera.worldToScreen(cm.position.x, cm.position.y, ground_y, viewport_w, viewport_h);
    const SDL_FPoint ground_s = camera.worldToScreen(cm.position.x, ref_h,         ground_y, viewport_w, viewport_h);

    SDL_SetRenderDrawColor(renderer, 0, 170, 255, 220);

    // Dashed projection line
    if (cmConfig.show_projection_line) {
        constexpr float DASH = 8.f;
        const float y_top    = std::min(cm_s.y, ground_s.y);
        const float y_bot    = std::max(cm_s.y, ground_s.y);
        float y = y_top;
        while (y < y_bot) {
            SDL_RenderDrawLineF(renderer, cm_s.x, y, cm_s.x, std::min(y + DASH, y_bot));
            y += DASH * 2.f;
        }
    }

    // Projection dot
    if (cmConfig.show_projection_dot)
        drawFilledCircle(renderer, ground_s.x, ground_s.y, 4.f);

    // Target-height tick: horizontal bar at CM minus nominal_y (ground level under CM)
    if (cmConfig.show_target_height_tick) {
        const double     L         = charConfig.body_height_m / 5.0;
        const double     cm_offset = computeNominalY(L, standConfig.d_pref, charConfig.cm_pelvis_ratio);
        const SDL_FPoint tick_s    = camera.worldToScreen(cm.position.x, cm.position.y - cm_offset,
                                                           ground_y, viewport_w, viewport_h);
        SDL_SetRenderDrawColor(renderer, 0, 170, 255, 220);
        SDL_RenderDrawLineF(renderer, tick_s.x - 12.f, tick_s.y, tick_s.x + 12.f, tick_s.y);
    }

    // Acceleration vector (red, 1g = 1 grid square)
    {
        const int mode = cmConfig.accel_components;
        if (mode > 0) {
            const Vec2 a = cm.acceleration * accel_display_scale;
            const SDL_FPoint sh    = camera.worldToScreen(cm.position.x + a.x, cm.position.y,       ground_y, viewport_w, viewport_h);
            const SDL_FPoint sv    = camera.worldToScreen(cm.position.x,       cm.position.y + a.y, ground_y, viewport_w, viewport_h);
            const SDL_FPoint sfull = camera.worldToScreen(cm.position.x + a.x, cm.position.y + a.y, ground_y, viewport_w, viewport_h);
            if (mode == 1) drawComponentArrow(renderer, cm_s.x, cm_s.y, sh.x,    sh.y,    230, 50, 50);
            if (mode == 2) drawComponentArrow(renderer, cm_s.x, cm_s.y, sv.x,    sv.y,    230, 50, 50);
            if (mode == 3) drawComponentArrow(renderer, cm_s.x, cm_s.y, sfull.x, sfull.y, 230, 50, 50);
        }
    }

    // Velocity vector (green, 1 m/s = 1 m)
    {
        const int mode = cmConfig.velocity_components;
        if (mode > 0) {
            const Vec2& v  = cm.velocity;
            const SDL_FPoint sh    = camera.worldToScreen(cm.position.x + v.x, cm.position.y,       ground_y, viewport_w, viewport_h);
            const SDL_FPoint sv    = camera.worldToScreen(cm.position.x,       cm.position.y + v.y, ground_y, viewport_w, viewport_h);
            const SDL_FPoint sfull = camera.worldToScreen(cm.position.x + v.x, cm.position.y + v.y, ground_y, viewport_w, viewport_h);
            if (mode == 1) drawComponentArrow(renderer, cm_s.x, cm_s.y, sh.x,    sh.y,    50, 220, 50);
            if (mode == 2) drawComponentArrow(renderer, cm_s.x, cm_s.y, sv.x,    sv.y,    50, 220, 50);
            if (mode == 3) drawComponentArrow(renderer, cm_s.x, cm_s.y, sfull.x, sfull.y, 50, 220, 50);
        }
    }

    // Drag preview (set CM velocity via right-click drag)
    if (drag_active) {
        SDL_SetRenderDrawColor(renderer, 50, 220, 50, 180);
        SDL_RenderDrawLineF(renderer, cm_s.x, cm_s.y, drag_mouse_x, drag_mouse_y);
        drawArrowHead(renderer, cm_s.x, cm_s.y, drag_mouse_x, drag_mouse_y, 10.f);
    }

    // Step target — cyan cross at land_target while step is active
    if (step_plan.active) {
        const SDL_FPoint t_s = camera.worldToScreen(step_plan.land_target.x,
                                                    step_plan.land_target.y,
                                                    ground_y, viewport_w, viewport_h);
        SDL_SetRenderDrawColor(renderer, 0, 220, 220, 220);
        constexpr float ARM = 7.f;
        SDL_RenderDrawLineF(renderer, t_s.x - ARM, t_s.y, t_s.x + ARM, t_s.y);
        SDL_RenderDrawLineF(renderer, t_s.x, t_s.y - ARM, t_s.x, t_s.y + ARM);
    }

    // XCoM marker — vertical line at xi, ground level (magenta)
    if (cmConfig.show_xcom_line && feet_initialized && balance.omega0 > 0.0) {
        const SDL_FPoint xi_top = camera.worldToScreen(balance.xcom, cm.position.y, ground_y, viewport_w, viewport_h);
        const SDL_FPoint xi_bot = camera.worldToScreen(balance.xcom, ref_h,         ground_y, viewport_w, viewport_h);
        SDL_SetRenderDrawColor(renderer, 220, 50, 220, 200);
        SDL_RenderDrawLineF(renderer, xi_top.x, xi_top.y, xi_bot.x, xi_bot.y);
        drawFilledCircle(renderer, xi_bot.x, xi_bot.y, 4.f);
    }
}
