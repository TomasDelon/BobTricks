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

CharacterRenderer::ScreenSpacePose CharacterRenderer::computeScreenSpacePose(const Camera2D& camera,
                                                                             const CMState& cm,
                                                                             const CharacterState& character,
                                                                             const CharacterConfig& charConfig,
                                                                             double ground_y,
                                                                             int viewport_w,
                                                                             int viewport_h) const
{
    const double L = charConfig.body_height_m / 5.0;
    const Vec2 head_axis_world = character.head_center - character.torso_top;
    const double head_axis_len = head_axis_world.length();
    const Vec2 neck_dir_world = (head_axis_len > kEpsLength)
                              ? (head_axis_world / head_axis_len)
                              : Vec2{0.0, 1.0};
    const Vec2 neck_attach_world = character.head_center
                                 - neck_dir_world * character.head_radius;

    ScreenSpacePose pose;
    pose.cm = camera.worldToScreen(cm.position.x, cm.position.y, ground_y, viewport_w, viewport_h);
    pose.pelvis = camera.worldToScreen(character.pelvis.x, character.pelvis.y, ground_y, viewport_w, viewport_h);
    pose.torso_center = camera.worldToScreen(character.torso_center.x, character.torso_center.y, ground_y, viewport_w, viewport_h);
    pose.torso_top = camera.worldToScreen(character.torso_top.x, character.torso_top.y, ground_y, viewport_w, viewport_h);
    pose.head = camera.worldToScreen(character.head_center.x, character.head_center.y, ground_y, viewport_w, viewport_h);
    pose.neck = camera.worldToScreen(neck_attach_world.x, neck_attach_world.y, ground_y, viewport_w, viewport_h);
    pose.elbow_left = camera.worldToScreen(character.elbow_left.x, character.elbow_left.y, ground_y, viewport_w, viewport_h);
    pose.elbow_right = camera.worldToScreen(character.elbow_right.x, character.elbow_right.y, ground_y, viewport_w, viewport_h);
    pose.hand_left = camera.worldToScreen(character.hand_left.x, character.hand_left.y, ground_y, viewport_w, viewport_h);
    pose.hand_right = camera.worldToScreen(character.hand_right.x, character.hand_right.y, ground_y, viewport_w, viewport_h);
    pose.hand_left_target = camera.worldToScreen(character.hand_left_target.x, character.hand_left_target.y, ground_y, viewport_w, viewport_h);
    pose.hand_right_target = camera.worldToScreen(character.hand_right_target.x, character.hand_right_target.y, ground_y, viewport_w, viewport_h);
    pose.knee_left = camera.worldToScreen(character.knee_left.x, character.knee_left.y, ground_y, viewport_w, viewport_h);
    pose.knee_right = camera.worldToScreen(character.knee_right.x, character.knee_right.y, ground_y, viewport_w, viewport_h);
    pose.foot_left = camera.worldToScreen(character.foot_left.pos.x, character.foot_left.pos.y, ground_y, viewport_w, viewport_h);
    pose.foot_right = camera.worldToScreen(character.foot_right.pos.x, character.foot_right.pos.y, ground_y, viewport_w, viewport_h);

    const SDL_FPoint reach_s = camera.worldToScreen(character.pelvis.x + 2.0 * L,
                                                    character.pelvis.y,
                                                    ground_y, viewport_w, viewport_h);
    pose.reach_radius = std::abs(reach_s.x - pose.pelvis.x);

    const SDL_FPoint head_r_s = camera.worldToScreen(character.head_center.x + character.head_radius,
                                                     character.head_center.y,
                                                     ground_y, viewport_w, viewport_h);
    pose.head_radius = std::abs(head_r_s.x - pose.head.x);
    return pose;
}

void CharacterRenderer::renderSplinePass(SDL_Renderer* renderer,
                                         const Camera2D& camera,
                                         const CharacterState& character,
                                         const CharacterConfig& charConfig,
                                         const SplineRenderConfig& splineConfig,
                                         double ground_y,
                                         int viewport_w,
                                         int viewport_h) const
{
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
    if (splineConfig.show_legs) {
        renderSplineLeg(renderer, camera,
                        character.pelvis, back_knee, back_foot, 2,
                        splineConfig, ground_y, viewport_w, viewport_h);
    }
    if (splineConfig.show_arms) {
        renderSplineArm(renderer, camera,
                        character.torso_top, back_elbow, back_hand, 2,
                        splineConfig, ground_y, viewport_w, viewport_h);
    }
    if (splineConfig.show_torso) {
        renderSplineTorso(renderer, camera, character, splineConfig,
                          ground_y, viewport_w, viewport_h);
    }
    if (splineConfig.show_head) {
        renderSplineHead(renderer, camera, character, splineConfig,
                         ground_y, viewport_w, viewport_h);
    }
    if (splineConfig.show_legs) {
        renderSplineLeg(renderer, camera,
                        character.pelvis, front_knee, front_foot, 0,
                        splineConfig, ground_y, viewport_w, viewport_h);
    }
    if (splineConfig.show_arms) {
        renderSplineArm(renderer, camera,
                        character.torso_top, front_elbow, front_hand, 0,
                        splineConfig, ground_y, viewport_w, viewport_h);
    }
}

void CharacterRenderer::renderLegacyBody(SDL_Renderer* renderer,
                                         const CharacterState& character,
                                         const ScreenSpacePose& pose) const
{
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
    SDL_RenderDrawLineF(renderer, pose.torso_top.x, pose.torso_top.y, pose.elbow_left.x, pose.elbow_left.y);
    SDL_RenderDrawLineF(renderer, pose.elbow_left.x, pose.elbow_left.y, pose.hand_left.x, pose.hand_left.y);
    SDL_RenderDrawLineF(renderer, pose.torso_top.x, pose.torso_top.y, pose.elbow_right.x, pose.elbow_right.y);
    SDL_RenderDrawLineF(renderer, pose.elbow_right.x, pose.elbow_right.y, pose.hand_right.x, pose.hand_right.y);
    SDL_RenderDrawLineF(renderer, pose.pelvis.x, pose.pelvis.y, pose.torso_center.x, pose.torso_center.y);
    SDL_RenderDrawLineF(renderer, pose.torso_center.x, pose.torso_center.y, pose.torso_top.x, pose.torso_top.y);
    SDL_RenderDrawLineF(renderer, pose.torso_top.x, pose.torso_top.y, pose.neck.x, pose.neck.y);

    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawFilledCircle(renderer, pose.pelvis.x, pose.pelvis.y, 4.f);
    drawFilledCircle(renderer, pose.torso_center.x, pose.torso_center.y, 4.f);
    drawFilledCircle(renderer, pose.torso_top.x, pose.torso_top.y, 4.f);
    drawFilledCircle(renderer, pose.elbow_left.x, pose.elbow_left.y, 3.f);
    drawFilledCircle(renderer, pose.elbow_right.x, pose.elbow_right.y, 3.f);
    if (character.hand_left_pinned)
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
    else
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawFilledCircle(renderer, pose.hand_left.x, pose.hand_left.y, 3.f);
    if (character.hand_right_pinned)
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
    else
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawFilledCircle(renderer, pose.hand_right.x, pose.hand_right.y, 3.f);
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawCircleOutline(renderer, pose.head.x, pose.head.y, pose.head_radius);

    if (!character.feet_initialized) return;

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
    SDL_RenderDrawLineF(renderer, pose.pelvis.x, pose.pelvis.y, pose.knee_left.x, pose.knee_left.y);
    SDL_RenderDrawLineF(renderer, pose.knee_left.x, pose.knee_left.y, pose.foot_left.x, pose.foot_left.y);
    SDL_RenderDrawLineF(renderer, pose.pelvis.x, pose.pelvis.y, pose.knee_right.x, pose.knee_right.y);
    SDL_RenderDrawLineF(renderer, pose.knee_right.x, pose.knee_right.y, pose.foot_right.x, pose.foot_right.y);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);
    drawFilledCircle(renderer, pose.knee_left.x, pose.knee_left.y, 3.f);
    drawFilledCircle(renderer, pose.knee_right.x, pose.knee_right.y, 3.f);
}

void CharacterRenderer::renderDebugMarkersBeforeBody(SDL_Renderer* renderer,
                                                     const Camera2D& camera,
                                                     const CharacterState& character,
                                                     const CharacterConfig& charConfig,
                                                     const Terrain& terrain,
                                                     const ScreenSpacePose& pose,
                                                     double ground_y,
                                                     int viewport_w,
                                                     int viewport_h) const
{
    if (character.feet_initialized
        && character.foot_left.pinned  && character.foot_left.on_ground
        && character.foot_right.pinned && character.foot_right.on_ground)
    {
        const double x_left  = std::min(character.foot_left.pos.x, character.foot_right.pos.x);
        const double x_right = std::max(character.foot_left.pos.x, character.foot_right.pos.x);
        const double step    = 1.0 / camera.getPixelsPerMeter();

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

    if (charConfig.show_pelvis_reach_disk) {
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 48);
        drawFilledCircle(renderer, pose.pelvis.x, pose.pelvis.y, pose.reach_radius);
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 160);
        drawCircleOutline(renderer, pose.pelvis.x, pose.pelvis.y, pose.reach_radius);
    }
}

void CharacterRenderer::renderDebugMarkersAfterBody(SDL_Renderer* renderer,
                                                    const Camera2D& camera,
                                                    const CharacterState& character,
                                                    const ScreenSpacePose& pose,
                                                    double ground_y,
                                                    int viewport_w,
                                                    int viewport_h) const
{
    auto drawHandTarget = [&](bool pinned, const SDL_FPoint& target) {
        if (!pinned) return;
        constexpr float kHalfPx = 6.f;
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
        SDL_RenderDrawLineF(renderer, target.x - kHalfPx, target.y, target.x + kHalfPx, target.y);
        SDL_RenderDrawLineF(renderer, target.x, target.y - kHalfPx, target.x, target.y + kHalfPx);
    };
    drawHandTarget(character.hand_left_pinned, pose.hand_left_target);
    drawHandTarget(character.hand_right_pinned, pose.hand_right_target);

    if (character.feet_initialized) {
        auto drawFoot = [&](const FootState& foot, const SDL_FPoint& foot_pos) {
            if (foot.pinned)
                SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
            else
                SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            drawFilledCircle(renderer, foot_pos.x, foot_pos.y, 4.f);

            if (foot.on_ground) {
                SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
                drawFilledCircle(renderer, foot_pos.x, foot_pos.y, 2.f);
            } else if (foot.pinned) {
                SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
                drawFilledCircle(renderer, foot_pos.x, foot_pos.y, 2.f);
            }

            if (foot.pinned) {
                constexpr float kHalfPx = 8.f;
                const SDL_FPoint pinned_pos = camera.worldToScreen(
                    foot.pinned_pos.x, foot.pinned_pos.y,
                    ground_y, viewport_w, viewport_h);
                const float snx = static_cast<float>(foot.pinned_normal.x);
                const float sny = -static_cast<float>(foot.pinned_normal.y);
                const SDL_FPoint p0 = { pinned_pos.x - snx * kHalfPx, pinned_pos.y - sny * kHalfPx };
                const SDL_FPoint p1 = { pinned_pos.x + snx * kHalfPx, pinned_pos.y + sny * kHalfPx };
                SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
                SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
            }
        };

        drawFoot(character.foot_left, pose.foot_left);
        drawFoot(character.foot_right, pose.foot_right);
    }

    SDL_SetRenderDrawColor(renderer, 0, 170, 255, 255);
    drawFilledCircle(renderer, pose.cm.x, pose.cm.y, CM_RADIUS);
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
    const ScreenSpacePose pose = computeScreenSpacePose(camera, cm, character, charConfig,
                                                        ground_y, viewport_w, viewport_h);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (spline_only) {
        renderSplinePass(renderer, camera, character, charConfig, splineConfig,
                         ground_y, viewport_w, viewport_h);
        return;
    }

    if (splineConfig.enabled && splineConfig.draw_under_legacy)
        renderSplinePass(renderer, camera, character, charConfig, splineConfig,
                         ground_y, viewport_w, viewport_h);

    renderDebugMarkersBeforeBody(renderer, camera, character, charConfig, terrain, pose,
                                 ground_y, viewport_w, viewport_h);
    renderLegacyBody(renderer, character, pose);
    renderDebugMarkersAfterBody(renderer, camera, character, pose,
                                ground_y, viewport_w, viewport_h);

    if (splineConfig.enabled && !splineConfig.draw_under_legacy)
        renderSplinePass(renderer, camera, character, charConfig, splineConfig,
                         ground_y, viewport_w, viewport_h);
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

    StrokePath path;
    const Vec2 handle0 = pelvis + (torso_center - pelvis) * 0.85;
    const Vec2 handle1 = neck - (neck - torso_top) * 0.85;
    path.moveTo(pelvis);
    path.cubicTo(handle0, handle1, neck);

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
