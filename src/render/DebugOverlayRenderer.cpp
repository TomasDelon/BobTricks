#include "render/DebugOverlayRenderer.h"
#include "core/math/MathConstants.h"
#include "core/physics/Geometry.h"

#include <cmath>
#include <algorithm>

namespace {

constexpr double MAX_CM_DRAG_SPEED_MPS = 10.0;

Vec2 armCirclePoint(Vec2 center, Vec2 body_right, Vec2 body_up, double radius, double angle_deg)
{
    const double a = angle_deg * kDegToRad;
    return center + body_right * (std::cos(a) * radius)
                  + body_up    * (std::sin(a) * radius);
}
} // namespace

void DebugOverlayRenderer::drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius)
{
    const int ir = static_cast<int>(std::ceil(radius));
    for (int dy = -ir; dy <= ir; ++dy) {
        const float fy = static_cast<float>(dy);
        const float dx = std::sqrt(std::max(0.f, radius * radius - fy * fy));
        SDL_RenderDrawLineF(renderer, cx - dx, cy + fy, cx + dx, cy + fy);
    }
}

void DebugOverlayRenderer::drawCircleOutline(SDL_Renderer* renderer, float cx, float cy, float radius)
{
    constexpr int SEGMENTS = 96;
    float prev_x = cx + radius;
    float prev_y = cy;
    for (int i = 1; i <= SEGMENTS; ++i) {
        const float a = static_cast<float>(kTau * i / SEGMENTS);
        const float x = cx + radius * std::cos(a);
        const float y = cy + radius * std::sin(a);
        SDL_RenderDrawLineF(renderer, prev_x, prev_y, x, y);
        prev_x = x;
        prev_y = y;
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

void DebugOverlayRenderer::renderXCoM(SDL_Renderer*   renderer,
                                       const Camera2D& camera,
                                       double          xi,
                                       double          target_x,
                                       bool            trigger,
                                       bool            show_target,
                                       const Terrain&  terrain,
                                       double          ground_y,
                                       int             viewport_w,
                                       int             viewport_h) const
{
    // ── ξ — vertical line upward from terrain, 30 px ─────────────────────────
    {
        const double     gy   = terrain.height_at(xi);
        const SDL_FPoint base = camera.worldToScreen(xi, gy, ground_y, viewport_w, viewport_h);

        if (trigger)
            SDL_SetRenderDrawColor(renderer, 230, 50, 50, 220);
        else
            SDL_SetRenderDrawColor(renderer, 0, 200, 255, 220);

        SDL_RenderDrawLineF(renderer, base.x, base.y, base.x, base.y - 30.f);
    }

    // ── Target palito — perpendicular to terrain normal, 8 px each side ──────
    if (show_target) {
        const double     ty  = terrain.height_at(target_x);
        const Vec2       n   = terrain.normal_at(target_x);
        const SDL_FPoint ps  = camera.worldToScreen(target_x, ty, ground_y, viewport_w, viewport_h);

        constexpr float kHalfPx = 8.f;
        const float snx =  static_cast<float>(n.x);
        const float sny = -static_cast<float>(n.y);
        const SDL_FPoint p0 = { ps.x - snx * kHalfPx, ps.y - sny * kHalfPx };
        const SDL_FPoint p1 = { ps.x + snx * kHalfPx, ps.y + sny * kHalfPx };

        if (trigger)
            SDL_SetRenderDrawColor(renderer, 230, 50, 50, 200);
        else
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);

        SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
    }
}

void DebugOverlayRenderer::renderBackground(SDL_Renderer*                 renderer,
                                             const Camera2D&               camera,
                                             const CMConfig&               cmConfig,
                                             const std::deque<TrailPoint>& trail,
                                             double                        sim_time,
                                             double                        ground_y,
                                             int                           viewport_w,
                                             int                           viewport_h) const
{
    if (!cmConfig.show_trail || trail.size() < 2) return;

    const double inv_dur = 1.0 / cmConfig.trail_duration;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (std::size_t i = 0; i + 1 < trail.size(); ++i) {
        const double age = sim_time - trail[i].time;
        const Uint8  a   = static_cast<Uint8>((1.0 - age * inv_dur) * 220.0);
        const SDL_FPoint p0 = camera.worldToScreen(trail[i].pos.x,   trail[i].pos.y,
                                                    ground_y, viewport_w, viewport_h);
        const SDL_FPoint p1 = camera.worldToScreen(trail[i+1].pos.x, trail[i+1].pos.y,
                                                    ground_y, viewport_w, viewport_h);
        SDL_SetRenderDrawColor(renderer, 0, 170, 255, a);
        SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
    }
}

void DebugOverlayRenderer::renderForeground(SDL_Renderer*          renderer,
                                             const Camera2D&        camera,
                                             const CMState&         cm,
                                             const CharacterState&  character,
                                             const CharacterConfig& charConfig,
                                             const HeadConfig&      headConfig,
                                             const ArmConfig&       armConfig,
                                             const StandingConfig&  standConfig,
                                             const CMConfig&        cmConfig,
                                             const Terrain&,
                                             const std::optional<Vec2>& gaze_target_world,
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

    // Ground reference line: use endpoints already clamped by physics (exact pelvis + disk).
    if (cmConfig.show_ground_reference) {
        const Vec2& back = character.debug_ground_back;
        const Vec2& fwd  = character.debug_ground_fwd;
        const SDL_FPoint pb = camera.worldToScreen(back.x, back.y, ground_y, viewport_w, viewport_h);
        const SDL_FPoint pf = camera.worldToScreen(fwd.x,  fwd.y,  ground_y, viewport_w, viewport_h);
        SDL_SetRenderDrawColor(renderer, 80, 220, 80, 220);
        SDL_RenderDrawLineF(renderer, pb.x, pb.y, pf.x, pf.y);
        drawFilledCircle(renderer, pb.x, pb.y, 3.f);
        drawFilledCircle(renderer, pf.x, pf.y, 3.f);
    }

    SDL_SetRenderDrawColor(renderer, 0, 170, 255, 220);

    // Dashed projection line
    if (cmConfig.show_projection_line) {
        constexpr float DASH = 8.f;
        const float y_top = std::min(cm_s.y, ground_s.y);
        const float y_bot = std::max(cm_s.y, ground_s.y);
        float y = y_top;
        while (y < y_bot) {
            SDL_RenderDrawLineF(renderer, cm_s.x, y, cm_s.x, std::min(y + DASH, y_bot));
            y += DASH * 2.f;
        }
    }

    // Projection dot
    if (cmConfig.show_projection_dot)
        drawFilledCircle(renderer, ground_s.x, ground_s.y, 4.f);

    // Target-height tick
    if (cmConfig.show_target_height_tick) {
        const double     L         = charConfig.body_height_m / 5.0;
        const double     cm_offset = computeNominalY(L, standConfig.d_pref, charConfig.cm_pelvis_ratio);
        const SDL_FPoint tick_s    = camera.worldToScreen(cm.position.x,
                                                           cm.position.y - cm_offset,
                                                           ground_y, viewport_w, viewport_h);
        SDL_SetRenderDrawColor(renderer, 0, 170, 255, 220);
        SDL_RenderDrawLineF(renderer, tick_s.x - 12.f, tick_s.y, tick_s.x + 12.f, tick_s.y);
    }

    // Acceleration vector (red)
    {
        const int mode = cmConfig.accel_components;
        if (mode > 0) {
            const Vec2       a     = cm.acceleration * accel_display_scale;
            const SDL_FPoint sh    = camera.worldToScreen(cm.position.x + a.x, cm.position.y,       ground_y, viewport_w, viewport_h);
            const SDL_FPoint sv    = camera.worldToScreen(cm.position.x,       cm.position.y + a.y, ground_y, viewport_w, viewport_h);
            const SDL_FPoint sfull = camera.worldToScreen(cm.position.x + a.x, cm.position.y + a.y, ground_y, viewport_w, viewport_h);
            if (mode == 1) drawComponentArrow(renderer, cm_s.x, cm_s.y, sh.x,    sh.y,    230, 50, 50);
            if (mode == 2) drawComponentArrow(renderer, cm_s.x, cm_s.y, sv.x,    sv.y,    230, 50, 50);
            if (mode == 3) drawComponentArrow(renderer, cm_s.x, cm_s.y, sfull.x, sfull.y, 230, 50, 50);
        }
    }

    // Velocity vector (green)
    {
        const int mode = cmConfig.velocity_components;
        if (mode > 0) {
            const Vec2&      v     = cm.velocity;
            const SDL_FPoint sh    = camera.worldToScreen(cm.position.x + v.x, cm.position.y,       ground_y, viewport_w, viewport_h);
            const SDL_FPoint sv    = camera.worldToScreen(cm.position.x,       cm.position.y + v.y, ground_y, viewport_w, viewport_h);
            const SDL_FPoint sfull = camera.worldToScreen(cm.position.x + v.x, cm.position.y + v.y, ground_y, viewport_w, viewport_h);
            if (mode == 1) drawComponentArrow(renderer, cm_s.x, cm_s.y, sh.x,    sh.y,    50, 220, 50);
            if (mode == 2) drawComponentArrow(renderer, cm_s.x, cm_s.y, sv.x,    sv.y,    50, 220, 50);
            if (mode == 3) drawComponentArrow(renderer, cm_s.x, cm_s.y, sfull.x, sfull.y, 50, 220, 50);
        }
    }

    // Drag preview (right-click set velocity)
    if (drag_active) {
        const Vec2 drag_world = camera.screenToWorld(
            drag_mouse_x, drag_mouse_y, ground_y, viewport_w, viewport_h);
        Vec2 drag_velocity = drag_world - cm.position;
        const double speed = drag_velocity.length();
        if (speed > MAX_CM_DRAG_SPEED_MPS)
            drag_velocity = drag_velocity * (MAX_CM_DRAG_SPEED_MPS / speed);
        const SDL_FPoint clamped_drag_s = camera.worldToScreen(
            cm.position.x + drag_velocity.x,
            cm.position.y + drag_velocity.y,
            ground_y, viewport_w, viewport_h);

        SDL_SetRenderDrawColor(renderer, 50, 220, 50, 180);
        SDL_RenderDrawLineF(renderer, cm_s.x, cm_s.y, clamped_drag_s.x, clamped_drag_s.y);
        drawArrowHead(renderer, cm_s.x, cm_s.y, clamped_drag_s.x, clamped_drag_s.y, 10.f);
    }

    if (gaze_target_world.has_value()
        && (headConfig.show_eye_marker || headConfig.show_gaze_ray || headConfig.show_gaze_target)) {
        const SDL_FPoint gaze_s = camera.worldToScreen(
            gaze_target_world->x, gaze_target_world->y,
            ground_y, viewport_w, viewport_h);
        const SDL_FPoint eye_s = camera.worldToScreen(
            character.eye_left.x, character.eye_left.y,
            ground_y, viewport_w, viewport_h);

        SDL_SetRenderDrawColor(renderer, 255, 120, 120, 210);
        if (headConfig.show_gaze_ray)
            SDL_RenderDrawLineF(renderer, eye_s.x, eye_s.y, gaze_s.x, gaze_s.y);
        if (headConfig.show_eye_marker)
            drawFilledCircle(renderer, eye_s.x, eye_s.y, 3.f);
        if (headConfig.show_gaze_target) {
            drawFilledCircle(renderer, gaze_s.x, gaze_s.y, 4.f);
            SDL_SetRenderDrawColor(renderer, 255, 120, 120, 255);
            drawCircleOutline(renderer, gaze_s.x, gaze_s.y, 7.f);
        }
    }

    const double L = charConfig.body_height_m / 5.0;
    const Vec2 body_up = normalizeOr(character.torso_top - character.torso_center, {0.0, 1.0});
    Vec2 body_right = {body_up.y, -body_up.x};
    if (body_right.x * character.facing < 0.0)
        body_right = body_right * -1.0;

    const Vec2 center = character.torso_top;
    const double outer_radius = 2.0 * L;
    const double inner_radius = std::max(0.05 * L, 2.0 * L - armConfig.walk_hand_reach_reduction_L * L);
    const SDL_FPoint center_s = camera.worldToScreen(center.x, center.y, ground_y, viewport_w, viewport_h);
    const SDL_FPoint outer_s  = camera.worldToScreen(center.x + outer_radius, center.y, ground_y, viewport_w, viewport_h);
    const SDL_FPoint inner_s  = camera.worldToScreen(center.x + inner_radius, center.y, ground_y, viewport_w, viewport_h);
    const float outer_radius_s = std::abs(outer_s.x - center_s.x);
    const float inner_radius_s = std::abs(inner_s.x - center_s.x);

    if (armConfig.show_debug_reach_circles) {
        SDL_SetRenderDrawColor(renderer, 160, 160, 160, 110);
        drawCircleOutline(renderer, center_s.x, center_s.y, outer_radius_s);
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 140);
        drawCircleOutline(renderer, center_s.x, center_s.y, inner_radius_s);
    }

    const Vec2 front_start = armCirclePoint(center, body_right, body_up, inner_radius, armConfig.walk_front_hand_start_deg);
    const Vec2 front_end   = armCirclePoint(center, body_right, body_up, inner_radius, armConfig.walk_front_hand_end_deg);
    const Vec2 back_start  = armCirclePoint(center, body_right, body_up, inner_radius, armConfig.walk_back_hand_start_deg);
    const Vec2 back_end    = armCirclePoint(center, body_right, body_up, inner_radius, armConfig.walk_back_hand_end_deg);

    auto toScreen = [&](Vec2 p) {
        return camera.worldToScreen(p.x, p.y, ground_y, viewport_w, viewport_h);
    };
    const SDL_FPoint front_start_s = toScreen(front_start);
    const SDL_FPoint front_end_s   = toScreen(front_end);
    const SDL_FPoint back_start_s  = toScreen(back_start);
    const SDL_FPoint back_end_s    = toScreen(back_end);

    auto drawArc = [&](double start_deg, double end_deg, Uint8 r, Uint8 g, Uint8 b) {
        constexpr int SEGMENTS = 32;
        SDL_SetRenderDrawColor(renderer, r, g, b, 180);
        Vec2 p0 = armCirclePoint(center, body_right, body_up, inner_radius, start_deg);
        SDL_FPoint prev = toScreen(p0);
        for (int i = 1; i <= SEGMENTS; ++i) {
            const double t = static_cast<double>(i) / static_cast<double>(SEGMENTS);
            const double angle = start_deg + (end_deg - start_deg) * t;
            const Vec2 p = armCirclePoint(center, body_right, body_up, inner_radius, angle);
            const SDL_FPoint cur = toScreen(p);
            SDL_RenderDrawLineF(renderer, prev.x, prev.y, cur.x, cur.y);
            prev = cur;
        }
    };

    if (armConfig.show_debug_swing_arcs) {
        drawArc(armConfig.walk_front_hand_start_deg, armConfig.walk_front_hand_end_deg, 255, 210, 60);
        drawArc(armConfig.walk_back_hand_start_deg, armConfig.walk_back_hand_end_deg, 60, 210, 255);
    }

    if (armConfig.show_debug_swing_points) {
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
        drawFilledCircle(renderer, front_start_s.x, front_start_s.y, 3.f);
        drawFilledCircle(renderer, front_end_s.x, front_end_s.y, 3.f);
        SDL_SetRenderDrawColor(renderer, 60, 210, 255, 255);
        drawFilledCircle(renderer, back_start_s.x, back_start_s.y, 3.f);
        drawFilledCircle(renderer, back_end_s.x, back_end_s.y, 3.f);
    }
}
