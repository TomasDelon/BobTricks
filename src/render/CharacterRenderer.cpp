#include "render/CharacterRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "core/math/MathConstants.h"
#include "core/math/StrokePath.h"

static constexpr float  CM_RADIUS    = 4.f;
static constexpr double CIRCLE_KAPPA = 0.5522847498307936;

namespace {

SDL_Color splineDepthColor(int depth_index)
{
    const SDL_Color base{0xF6, 0x6C, 0x00, 0xFF};
    const int clamped = std::max(0, std::min(depth_index, 2));
    const float darken = 0.14f * static_cast<float>(clamped);
    auto apply = [&](std::uint8_t c) -> std::uint8_t {
        const float v = static_cast<float>(c) * (1.0f - darken);
        return static_cast<std::uint8_t>(std::max(0.0f, std::min(255.0f, v)));
    };
    return SDL_Color{apply(base.r), apply(base.g), apply(base.b), base.a};
}

} // namespace

void CharacterRenderer::drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius)
{
    const int ir = static_cast<int>(std::ceil(radius));
    for (int dy = -ir; dy <= ir; ++dy) {
        const float fy = static_cast<float>(dy);
        const float dx = std::sqrt(std::max(0.f, radius * radius - fy * fy));
        SDL_RenderDrawLineF(renderer, cx - dx, cy + fy, cx + dx, cy + fy);
    }
}

void CharacterRenderer::drawCircleOutline(SDL_Renderer* renderer, float cx, float cy, float radius)
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

void CharacterRenderer::render(SDL_Renderer*         renderer,
                               const Camera2D&       camera,
                               const CMState&        cm,
                               const CharacterState& character,
                               const CharacterConfig& charConfig,
                               const SplineRenderConfig& splineConfig,
                               const CharacterReconstructionConfig& /*reconstruction*/,
                               const CMConfig&       /*cmConfig*/,
                               bool                  spline_only,
                               const Terrain&        terrain,
                               double                ground_y,
                               int                   viewport_w,
                               int                   viewport_h) const
{
    const double L  = charConfig.body_height_m / 5.0;
    const Vec2 head_axis_world = character.head_center - character.torso_top;
    const double head_axis_len = head_axis_world.length();
    const Vec2 neck_dir_world = (head_axis_len > kEpsLength)
                              ? (head_axis_world / head_axis_len)
                              : Vec2{0.0, 1.0};
    const Vec2 neck_attach_world = character.head_center
                                 - neck_dir_world * character.head_radius;
    const SDL_FPoint cm_s     = camera.worldToScreen(cm.position.x,   cm.position.y,   ground_y, viewport_w, viewport_h);
    const SDL_FPoint pelvis_s = camera.worldToScreen(character.pelvis.x,       character.pelvis.y,       ground_y, viewport_w, viewport_h);
    const SDL_FPoint tc_s     = camera.worldToScreen(character.torso_center.x, character.torso_center.y, ground_y, viewport_w, viewport_h);
    const SDL_FPoint tt_s     = camera.worldToScreen(character.torso_top.x,    character.torso_top.y,    ground_y, viewport_w, viewport_h);
    const SDL_FPoint head_s   = camera.worldToScreen(character.head_center.x,   character.head_center.y,  ground_y, viewport_w, viewport_h);
    const SDL_FPoint neck_s   = camera.worldToScreen(neck_attach_world.x,       neck_attach_world.y,     ground_y, viewport_w, viewport_h);
    const SDL_FPoint el_s     = camera.worldToScreen(character.elbow_left.x,     character.elbow_left.y,     ground_y, viewport_w, viewport_h);
    const SDL_FPoint er_s     = camera.worldToScreen(character.elbow_right.x,    character.elbow_right.y,    ground_y, viewport_w, viewport_h);
    const SDL_FPoint hl_s     = camera.worldToScreen(character.hand_left.x,      character.hand_left.y,      ground_y, viewport_w, viewport_h);
    const SDL_FPoint hr_s     = camera.worldToScreen(character.hand_right.x,     character.hand_right.y,     ground_y, viewport_w, viewport_h);
    const SDL_FPoint hlt_s    = camera.worldToScreen(character.hand_left_target.x, character.hand_left_target.y, ground_y, viewport_w, viewport_h);
    const SDL_FPoint hrt_s    = camera.worldToScreen(character.hand_right_target.x, character.hand_right_target.y, ground_y, viewport_w, viewport_h);
    const SDL_FPoint reach_s  = camera.worldToScreen(character.pelvis.x + 2.0 * L,
                                                     character.pelvis.y,
                                                     ground_y, viewport_w, viewport_h);
    const float reach_radius_s = std::abs(reach_s.x - pelvis_s.x);
    const SDL_FPoint head_r_s = camera.worldToScreen(character.head_center.x + character.head_radius,
                                                     character.head_center.y,
                                                     ground_y, viewport_w, viewport_h);
    const float head_radius_s = std::abs(head_r_s.x - head_s.x);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    auto renderSplinePass = [&]() {
        if (!splineConfig.enabled) return;
        if (splineConfig.show_test_curve) {
            renderSplineTest(renderer, camera, character, charConfig, splineConfig,
                             ground_y, viewport_w, viewport_h);
        }
        const bool facing_right = character.facing >= 0.0;
        const Vec2& back_knee   = facing_right ? character.knee_right : character.knee_left;
        const Vec2& back_foot   = facing_right ? character.foot_right.pos : character.foot_left.pos;
        const Vec2& front_knee  = facing_right ? character.knee_left  : character.knee_right;
        const Vec2& front_foot  = facing_right ? character.foot_left.pos : character.foot_right.pos;
        const Vec2& back_elbow  = facing_right ? character.elbow_right : character.elbow_left;
        const Vec2& back_hand   = facing_right ? character.hand_right  : character.hand_left;
        const Vec2& front_elbow = facing_right ? character.elbow_left  : character.elbow_right;
        const Vec2& front_hand  = facing_right ? character.hand_left   : character.hand_right;
        renderSplineLeg(renderer, camera,
                        character.pelvis, back_knee, back_foot, 2,
                        splineConfig, ground_y, viewport_w, viewport_h);
        renderSplineArm(renderer, camera,
                        character.torso_top, back_elbow, back_hand, 2,
                        splineConfig, ground_y, viewport_w, viewport_h);
        renderSplineTorso(renderer, camera, character, splineConfig,
                          ground_y, viewport_w, viewport_h);
        renderSplineHead(renderer, camera, character, splineConfig,
                         ground_y, viewport_w, viewport_h);
        renderSplineLeg(renderer, camera,
                        character.pelvis, front_knee, front_foot, 0,
                        splineConfig, ground_y, viewport_w, viewport_h);
        renderSplineArm(renderer, camera,
                        character.torso_top, front_elbow, front_hand, 0,
                        splineConfig, ground_y, viewport_w, viewport_h);
    };

    if (spline_only) {
        renderSplinePass();
        return;
    }

    if (splineConfig.enabled && splineConfig.draw_under_legacy)
        renderSplinePass();

    // Support interval: orange terrain segment between feet when both pinned + on_ground.
    // Sample the terrain at 1-pixel steps between the two foot x-positions.
    if (character.feet_initialized
        && character.foot_left.pinned  && character.foot_left.on_ground
        && character.foot_right.pinned && character.foot_right.on_ground)
    {
        const double x_left  = std::min(character.foot_left.pos.x, character.foot_right.pos.x);
        const double x_right = std::max(character.foot_left.pos.x, character.foot_right.pos.x);
        const double step    = 1.0 / camera.getPixelsPerMeter();  // 1 px in world units

        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
        SDL_FPoint prev = camera.worldToScreen(x_left, terrain.height_at(x_left),
                                               ground_y, viewport_w, viewport_h);
        for (double x = x_left + step; x <= x_right + step * 0.5; x += step) {
            const double cx = std::min(x, x_right);
            const SDL_FPoint cur = camera.worldToScreen(cx, terrain.height_at(cx),
                                                        ground_y, viewport_w, viewport_h);
            SDL_RenderDrawLineF(renderer, prev.x, prev.y, cur.x, cur.y);
            prev = cur;
        }
    }

    // Reach disk centered at pelvis with radius 2L.
    if (charConfig.show_pelvis_reach_disk) {
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 48);
        drawFilledCircle(renderer, pelvis_s.x, pelvis_s.y, reach_radius_s);
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 160);
        drawCircleOutline(renderer, pelvis_s.x, pelvis_s.y, reach_radius_s);
    }

    // Torso spine — reconstructed locally from CM and theta.
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
    SDL_RenderDrawLineF(renderer, tt_s.x, tt_s.y, el_s.x, el_s.y);
    SDL_RenderDrawLineF(renderer, el_s.x, el_s.y, hl_s.x, hl_s.y);
    SDL_RenderDrawLineF(renderer, tt_s.x, tt_s.y, er_s.x, er_s.y);
    SDL_RenderDrawLineF(renderer, er_s.x, er_s.y, hr_s.x, hr_s.y);
    SDL_RenderDrawLineF(renderer, pelvis_s.x, pelvis_s.y, tc_s.x, tc_s.y);
    SDL_RenderDrawLineF(renderer, tc_s.x, tc_s.y, tt_s.x, tt_s.y);
    SDL_RenderDrawLineF(renderer, tt_s.x, tt_s.y, neck_s.x, neck_s.y);

    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawFilledCircle(renderer, pelvis_s.x, pelvis_s.y, 4.f);
    drawFilledCircle(renderer, tc_s.x,     tc_s.y,     4.f);
    drawFilledCircle(renderer, tt_s.x,     tt_s.y,     4.f);
    drawFilledCircle(renderer, el_s.x,     el_s.y,     3.f);
    drawFilledCircle(renderer, er_s.x,     er_s.y,     3.f);
    if (character.hand_left_pinned)
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
    else
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawFilledCircle(renderer, hl_s.x,     hl_s.y,     3.f);
    if (character.hand_right_pinned)
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
    else
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawFilledCircle(renderer, hr_s.x,     hr_s.y,     3.f);
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawCircleOutline(renderer, head_s.x,  head_s.y,   head_radius_s);

    auto drawHandTarget = [&](bool pinned, const SDL_FPoint& ps) {
        if (!pinned) return;
        constexpr float kHalfPx = 6.f;
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
        SDL_RenderDrawLineF(renderer, ps.x - kHalfPx, ps.y, ps.x + kHalfPx, ps.y);
        SDL_RenderDrawLineF(renderer, ps.x, ps.y - kHalfPx, ps.x, ps.y + kHalfPx);
    };
    drawHandTarget(character.hand_left_pinned, hlt_s);
    drawHandTarget(character.hand_right_pinned, hrt_s);

    if (character.feet_initialized) {
        const SDL_FPoint lf_s = camera.worldToScreen(character.foot_left.pos.x, character.foot_left.pos.y,
                                                     ground_y, viewport_w, viewport_h);
        const SDL_FPoint rf_s = camera.worldToScreen(character.foot_right.pos.x, character.foot_right.pos.y,
                                                     ground_y, viewport_w, viewport_h);
        const SDL_FPoint lk_s = camera.worldToScreen(character.knee_left.x, character.knee_left.y,
                                                     ground_y, viewport_w, viewport_h);
        const SDL_FPoint rk_s = camera.worldToScreen(character.knee_right.x, character.knee_right.y,
                                                     ground_y, viewport_w, viewport_h);

        // Leg segments: pelvis → knee → foot
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
        SDL_RenderDrawLineF(renderer, pelvis_s.x, pelvis_s.y, lk_s.x, lk_s.y);
        SDL_RenderDrawLineF(renderer, lk_s.x, lk_s.y, lf_s.x, lf_s.y);
        SDL_RenderDrawLineF(renderer, pelvis_s.x, pelvis_s.y, rk_s.x, rk_s.y);
        SDL_RenderDrawLineF(renderer, rk_s.x, rk_s.y, rf_s.x, rf_s.y);

        // Knee dots
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);
        drawFilledCircle(renderer, lk_s.x, lk_s.y, 3.f);
        drawFilledCircle(renderer, rk_s.x, rk_s.y, 3.f);

        // Foot dots + normal line — drawn per foot
        auto drawFoot = [&](const FootState& foot, const SDL_FPoint& fs) {
            // Big circle: orange if pinned, white otherwise
            if (foot.pinned)
                SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
            else
                SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            drawFilledCircle(renderer, fs.x, fs.y, 4.f);

            // Small inner circle: orange if touching, white if pinned-but-not-touching, absent otherwise
            if (foot.on_ground) {
                SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
                drawFilledCircle(renderer, fs.x, fs.y, 2.f);
            } else if (foot.pinned) {
                SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
                drawFilledCircle(renderer, fs.x, fs.y, 2.f);
            }

            // Target stick when pinned (screen-space, zoom-independent)
            if (foot.pinned) {
                constexpr float kHalfPx = 8.f;
                const SDL_FPoint ps = camera.worldToScreen(
                    foot.pinned_pos.x, foot.pinned_pos.y,
                    ground_y, viewport_w, viewport_h);
                const float snx =  static_cast<float>(foot.pinned_normal.x);
                const float sny = -static_cast<float>(foot.pinned_normal.y);
                const SDL_FPoint p0 = { ps.x - snx * kHalfPx, ps.y - sny * kHalfPx };
                const SDL_FPoint p1 = { ps.x + snx * kHalfPx, ps.y + sny * kHalfPx };
                SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
                SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
            }
        };

        drawFoot(character.foot_left,  lf_s);
        drawFoot(character.foot_right, rf_s);
    }

    SDL_SetRenderDrawColor(renderer, 0, 170, 255, 255);
    drawFilledCircle(renderer, cm_s.x, cm_s.y, CM_RADIUS);

    if (splineConfig.enabled && !splineConfig.draw_under_legacy)
        renderSplinePass();
}

void CharacterRenderer::renderSplineTest(SDL_Renderer* renderer,
                                         const Camera2D& camera,
                                         const CharacterState& character,
                                         const CharacterConfig& charConfig,
                                         const SplineRenderConfig& splineConfig,
                                         double ground_y,
                                         int viewport_w,
                                         int viewport_h) const
{
    const double L = charConfig.body_height_m / 5.0;
    const Vec2 base = character.torso_top + Vec2{-1.35 * L, 0.35 * L};

    StrokePath path;
    path.moveTo(base);
    path.cubicTo(base + Vec2{0.35 * L, 0.85 * L},
                 base + Vec2{0.95 * L, -0.10 * L},
                 base + Vec2{1.30 * L, 0.55 * L});

    const std::vector<Vec2> world_points = path.flatten(splineConfig.samples_per_curve);
    std::vector<SDL_FPoint> screen_points;
    screen_points.reserve(world_points.size());
    for (const Vec2& p : world_points)
        screen_points.push_back(camera.worldToScreen(p.x, p.y, ground_y, viewport_w, viewport_h));

    m_strokeRenderer.renderPolyline(renderer, screen_points,
                                    splineConfig.stroke_width_px,
                                    splineDepthColor(0));

    if (splineConfig.show_control_polygon) {
        const std::vector<Vec2> controls = path.controlPolygon();
        SDL_SetRenderDrawColor(renderer, 255, 170, 40, 200);
        for (std::size_t i = 1; i < controls.size(); ++i) {
            const SDL_FPoint p0 = camera.worldToScreen(controls[i - 1].x, controls[i - 1].y,
                                                       ground_y, viewport_w, viewport_h);
            const SDL_FPoint p1 = camera.worldToScreen(controls[i].x, controls[i].y,
                                                       ground_y, viewport_w, viewport_h);
            SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
        }
        for (const Vec2& p : controls) {
            const SDL_FPoint ps = camera.worldToScreen(p.x, p.y, ground_y, viewport_w, viewport_h);
            SDL_SetRenderDrawColor(renderer, 255, 170, 40, 255);
            drawFilledCircle(renderer, ps.x, ps.y, 3.f);
        }
    }

    if (splineConfig.show_sample_points) {
        SDL_SetRenderDrawColor(renderer, 80, 255, 160, 255);
        for (const SDL_FPoint& p : screen_points)
            drawFilledCircle(renderer, p.x, p.y, 2.f);
    }
}

void CharacterRenderer::renderSplineHead(SDL_Renderer* renderer,
                                         const Camera2D& camera,
                                         const CharacterState& character,
                                         const SplineRenderConfig& splineConfig,
                                         double ground_y,
                                         int viewport_w,
                                         int viewport_h) const
{
    const double r = character.head_radius;
    const double k = CIRCLE_KAPPA * r;
    const Vec2 c = character.head_center;

    StrokePath path;
    path.moveTo({c.x + r, c.y});
    path.cubicTo({c.x + r, c.y + k},
                 {c.x + k, c.y + r},
                 {c.x,     c.y + r});
    path.cubicTo({c.x - k, c.y + r},
                 {c.x - r, c.y + k},
                 {c.x - r, c.y});
    path.cubicTo({c.x - r, c.y - k},
                 {c.x - k, c.y - r},
                 {c.x,     c.y - r});
    path.cubicTo({c.x + k, c.y - r},
                 {c.x + r, c.y - k},
                 {c.x + r, c.y});
    path.closePath();

    const std::vector<Vec2> world_points = path.flatten(splineConfig.samples_per_curve);
    std::vector<SDL_FPoint> screen_points;
    screen_points.reserve(world_points.size());
    for (const Vec2& p : world_points)
        screen_points.push_back(camera.worldToScreen(p.x, p.y, ground_y, viewport_w, viewport_h));

    m_strokeRenderer.renderPolyline(renderer, screen_points,
                                    splineConfig.stroke_width_px,
                                    splineDepthColor(0));

    if (splineConfig.show_control_polygon) {
        const std::vector<Vec2> controls = path.controlPolygon();
        SDL_SetRenderDrawColor(renderer, 255, 170, 40, 200);
        for (std::size_t i = 1; i < controls.size(); ++i) {
            const SDL_FPoint p0 = camera.worldToScreen(controls[i - 1].x, controls[i - 1].y,
                                                       ground_y, viewport_w, viewport_h);
            const SDL_FPoint p1 = camera.worldToScreen(controls[i].x, controls[i].y,
                                                       ground_y, viewport_w, viewport_h);
            SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
        }
    }

    if (splineConfig.show_sample_points) {
        SDL_SetRenderDrawColor(renderer, 80, 255, 160, 255);
        for (const SDL_FPoint& p : screen_points)
            drawFilledCircle(renderer, p.x, p.y, 2.f);
    }
}

void CharacterRenderer::renderSplineTorso(SDL_Renderer* renderer,
                                          const Camera2D& camera,
                                          const CharacterState& character,
                                          const SplineRenderConfig& splineConfig,
                                          double ground_y,
                                          int viewport_w,
                                          int viewport_h) const
{
    const Vec2 head_axis_world = character.head_center - character.torso_top;
    const double head_axis_len = head_axis_world.length();
    const Vec2 neck_dir_world = (head_axis_len > kEpsLength)
                              ? (head_axis_world / head_axis_len)
                              : Vec2{0.0, 1.0};
    const Vec2 neck_attach_world = character.head_center
                                 - neck_dir_world * character.head_radius;

    const Vec2 pelvis = character.pelvis;
    const Vec2 torso_center = character.torso_center;
    const Vec2 torso_top = character.torso_top;
    const Vec2 neck = neck_attach_world;

    const Vec2 handle0 = pelvis + (torso_center - pelvis) * 0.5;
    const Vec2 handle1 = torso_center - (torso_top - pelvis) * (1.0 / 6.0);
    const Vec2 handle2 = torso_center + (neck - pelvis) * (1.0 / 6.0);
    StrokePath path;
    path.moveTo(pelvis);
    path.cubicTo(handle0, handle1, torso_top);
    path.quadTo(handle2, neck);

    const std::vector<Vec2> world_points = path.flatten(splineConfig.samples_per_curve);
    std::vector<SDL_FPoint> screen_points;
    screen_points.reserve(world_points.size());
    for (const Vec2& p : world_points)
        screen_points.push_back(camera.worldToScreen(p.x, p.y, ground_y, viewport_w, viewport_h));

    m_strokeRenderer.renderPolyline(renderer, screen_points,
                                    splineConfig.stroke_width_px,
                                    splineDepthColor(1));

    if (splineConfig.show_control_polygon) {
        const std::vector<Vec2> controls = path.controlPolygon();
        SDL_SetRenderDrawColor(renderer, 255, 170, 40, 200);
        for (std::size_t i = 1; i < controls.size(); ++i) {
            const SDL_FPoint p0 = camera.worldToScreen(controls[i - 1].x, controls[i - 1].y,
                                                       ground_y, viewport_w, viewport_h);
            const SDL_FPoint p1 = camera.worldToScreen(controls[i].x, controls[i].y,
                                                       ground_y, viewport_w, viewport_h);
            SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
        }
    }

    if (splineConfig.show_sample_points) {
        SDL_SetRenderDrawColor(renderer, 80, 255, 160, 255);
        for (const SDL_FPoint& p : screen_points)
            drawFilledCircle(renderer, p.x, p.y, 2.f);
    }
}

void CharacterRenderer::renderSplineArm(SDL_Renderer* renderer,
                                        const Camera2D& camera,
                                        const Vec2& shoulder,
                                        const Vec2& elbow,
                                        const Vec2& hand,
                                        int depth_index,
                                        const SplineRenderConfig& splineConfig,
                                        double ground_y,
                                        int viewport_w,
                                        int viewport_h) const
{
    const Vec2 shoulder_to_elbow = elbow - shoulder;
    const Vec2 elbow_to_hand = hand - elbow;
    const Vec2 handle0 = shoulder + shoulder_to_elbow * (2.0 / 3.0);
    const Vec2 handle1 = hand - elbow_to_hand * (2.0 / 3.0);

    StrokePath path;
    path.moveTo(shoulder);
    path.cubicTo(handle0, handle1, hand);

    const std::vector<Vec2> world_points = path.flatten(splineConfig.samples_per_curve);
    std::vector<SDL_FPoint> screen_points;
    screen_points.reserve(world_points.size());
    for (const Vec2& p : world_points)
        screen_points.push_back(camera.worldToScreen(p.x, p.y, ground_y, viewport_w, viewport_h));

    m_strokeRenderer.renderPolyline(renderer, screen_points,
                                    splineConfig.stroke_width_px,
                                    splineDepthColor(depth_index));

    if (splineConfig.show_control_polygon) {
        const std::vector<Vec2> controls = path.controlPolygon();
        SDL_SetRenderDrawColor(renderer, 255, 170, 40, 200);
        for (std::size_t i = 1; i < controls.size(); ++i) {
            const SDL_FPoint p0 = camera.worldToScreen(controls[i - 1].x, controls[i - 1].y,
                                                       ground_y, viewport_w, viewport_h);
            const SDL_FPoint p1 = camera.worldToScreen(controls[i].x, controls[i].y,
                                                       ground_y, viewport_w, viewport_h);
            SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
        }
    }

    if (splineConfig.show_sample_points) {
        SDL_SetRenderDrawColor(renderer, 80, 255, 160, 255);
        for (const SDL_FPoint& p : screen_points)
            drawFilledCircle(renderer, p.x, p.y, 2.f);
    }
}

void CharacterRenderer::renderSplineLeg(SDL_Renderer* renderer,
                                        const Camera2D& camera,
                                        const Vec2& pelvis,
                                        const Vec2& knee,
                                        const Vec2& foot,
                                        int depth_index,
                                        const SplineRenderConfig& splineConfig,
                                        double ground_y,
                                        int viewport_w,
                                        int viewport_h) const
{
    const Vec2 pelvis_to_knee = knee - pelvis;
    const Vec2 knee_to_foot = foot - knee;
    const Vec2 handle0 = pelvis + pelvis_to_knee * (2.0 / 3.0);
    const Vec2 handle1 = foot - knee_to_foot * (2.0 / 3.0);

    StrokePath path;
    path.moveTo(pelvis);
    path.cubicTo(handle0, handle1, foot);

    const std::vector<Vec2> world_points = path.flatten(splineConfig.samples_per_curve);
    std::vector<SDL_FPoint> screen_points;
    screen_points.reserve(world_points.size());
    for (const Vec2& p : world_points)
        screen_points.push_back(camera.worldToScreen(p.x, p.y, ground_y, viewport_w, viewport_h));

    m_strokeRenderer.renderPolyline(renderer, screen_points,
                                    splineConfig.stroke_width_px,
                                    splineDepthColor(depth_index));

    if (splineConfig.show_control_polygon) {
        const std::vector<Vec2> controls = path.controlPolygon();
        SDL_SetRenderDrawColor(renderer, 255, 170, 40, 200);
        for (std::size_t i = 1; i < controls.size(); ++i) {
            const SDL_FPoint p0 = camera.worldToScreen(controls[i - 1].x, controls[i - 1].y,
                                                       ground_y, viewport_w, viewport_h);
            const SDL_FPoint p1 = camera.worldToScreen(controls[i].x, controls[i].y,
                                                       ground_y, viewport_w, viewport_h);
            SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
        }
    }

    if (splineConfig.show_sample_points) {
        SDL_SetRenderDrawColor(renderer, 80, 255, 160, 255);
        for (const SDL_FPoint& p : screen_points)
            drawFilledCircle(renderer, p.x, p.y, 2.f);
    }
}
