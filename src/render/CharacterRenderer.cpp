#include "render/CharacterRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "core/math/MathConstants.h"
#include "core/math/StrokePath.h"

static constexpr float  CM_RADIUS    = 4.f;
static constexpr double CIRCLE_KAPPA = 0.5522847498307936;

namespace {

std::uint8_t darkenChannel(std::uint8_t channel, float darken)
{
    const float value = static_cast<float>(channel) * (1.0f - darken);
    return static_cast<std::uint8_t>(std::max(0.0f, std::min(255.0f, value)));
}

SDL_Color splineDepthColor(int depth_index)
{
    const SDL_Color base{0xF6, 0x6C, 0x00, 0xFF};
    const int clamped = std::max(0, std::min(depth_index, 2));
    const float darken = 0.14f * static_cast<float>(clamped);
    return SDL_Color{
        darkenChannel(base.r, darken),
        darkenChannel(base.g, darken),
        darkenChannel(base.b, darken),
        base.a
    };
}

void drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius)
{
    const int ir = static_cast<int>(std::ceil(radius));
    for (int dy = -ir; dy <= ir; ++dy) {
        const float fy = static_cast<float>(dy);
        const float dx = std::sqrt(std::max(0.f, radius * radius - fy * fy));
        SDL_RenderDrawLineF(renderer, cx - dx, cy + fy, cx + dx, cy + fy);
    }
}

void drawCircleOutline(SDL_Renderer* renderer, float cx, float cy, float radius)
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

float scaled(float value, float debug_scale)
{
    return value * std::max(1.0f, debug_scale);
}

SDL_FPoint worldToScreenPoint(const Camera2D& camera,
                              const Vec2& p,
                              double ground_y,
                              int viewport_w,
                              int viewport_h)
{
    return camera.worldToScreen(p.x, p.y, ground_y, viewport_w, viewport_h);
}

void drawDebugLine(SDL_Renderer* renderer,
                   float x0, float y0,
                   float x1, float y1,
                   float debug_scale)
{
    const int layers = std::max(1, static_cast<int>(std::round(std::max(1.0f, debug_scale))));
    const float dx = x1 - x0;
    const float dy = y1 - y0;
    const float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.5f) {
        SDL_RenderDrawLineF(renderer, x0, y0, x1, y1);
        return;
    }
    const float nx = -dy / len;
    const float ny =  dx / len;
    const float start = -0.5f * static_cast<float>(layers - 1);
    for (int i = 0; i < layers; ++i) {
        const float off = start + static_cast<float>(i);
        SDL_RenderDrawLineF(renderer, x0 + nx * off, y0 + ny * off,
                            x1 + nx * off, y1 + ny * off);
    }
}

void renderFlattenedPath(SDL_Renderer* renderer,
                         const StrokeRenderer& strokeRenderer,
                         const Camera2D& camera,
                         const StrokePath& path,
                         int depth_index,
                         const SplineRenderConfig& splineConfig,
                         double ground_y, int viewport_w, int viewport_h)
{
    const std::vector<Vec2> world_points = path.flatten(splineConfig.samples_per_curve);
    std::vector<SDL_FPoint> screen_points;
    screen_points.reserve(world_points.size());
    for (const Vec2& p : world_points)
        screen_points.push_back(camera.worldToScreen(p.x, p.y, ground_y, viewport_w, viewport_h));

    strokeRenderer.renderPolyline(renderer, screen_points,
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

void drawPinnableHand(SDL_Renderer* renderer,
                      const SDL_FPoint& p,
                      bool pinned,
                      float debug_scale)
{
    if (pinned) SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
    else        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawFilledCircle(renderer, p.x, p.y, scaled(3.f, debug_scale));
}

void drawHandTarget(SDL_Renderer* renderer,
                    bool pinned,
                    const SDL_FPoint& target,
                    float debug_scale)
{
    if (!pinned) return;
    const float half_px = scaled(6.f, debug_scale);
    SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
    drawDebugLine(renderer, target.x - half_px, target.y,
                  target.x + half_px, target.y, debug_scale);
    drawDebugLine(renderer, target.x, target.y - half_px,
                  target.x, target.y + half_px, debug_scale);
}

void drawFootMarker(SDL_Renderer* renderer,
                    const Camera2D& camera,
                    const FootState& foot,
                    const SDL_FPoint& foot_pos,
                    double ground_y,
                    int viewport_w,
                    int viewport_h,
                    float debug_scale)
{
    if (foot.pinned)
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
    else
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    drawFilledCircle(renderer, foot_pos.x, foot_pos.y, scaled(4.f, debug_scale));

    if (foot.on_ground) {
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
        drawFilledCircle(renderer, foot_pos.x, foot_pos.y, scaled(2.f, debug_scale));
    } else if (foot.pinned) {
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        drawFilledCircle(renderer, foot_pos.x, foot_pos.y, scaled(2.f, debug_scale));
    }

    if (foot.pinned) {
        const float half_px = scaled(8.f, debug_scale);
        const SDL_FPoint pinned_pos = camera.worldToScreen(
            foot.pinned_pos.x, foot.pinned_pos.y,
            ground_y, viewport_w, viewport_h);
        const float snx = static_cast<float>(foot.pinned_normal.x);
        const float sny = -static_cast<float>(foot.pinned_normal.y);
        const SDL_FPoint p0 = { pinned_pos.x - snx * half_px, pinned_pos.y - sny * half_px };
        const SDL_FPoint p1 = { pinned_pos.x + snx * half_px, pinned_pos.y + sny * half_px };
        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
        drawDebugLine(renderer, p0.x, p0.y, p1.x, p1.y, debug_scale);
    }
}

} // namespace

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
    pose.cm                = worldToScreenPoint(camera, cm.position, ground_y, viewport_w, viewport_h);
    pose.pelvis            = worldToScreenPoint(camera, character.pelvis, ground_y, viewport_w, viewport_h);
    pose.torso_center      = worldToScreenPoint(camera, character.torso_center, ground_y, viewport_w, viewport_h);
    pose.torso_top         = worldToScreenPoint(camera, character.torso_top, ground_y, viewport_w, viewport_h);
    pose.head              = worldToScreenPoint(camera, character.head_center, ground_y, viewport_w, viewport_h);
    pose.neck              = worldToScreenPoint(camera, neck_attach_world, ground_y, viewport_w, viewport_h);
    pose.elbow_left        = worldToScreenPoint(camera, character.elbow_left, ground_y, viewport_w, viewport_h);
    pose.elbow_right       = worldToScreenPoint(camera, character.elbow_right, ground_y, viewport_w, viewport_h);
    pose.hand_left         = worldToScreenPoint(camera, character.hand_left, ground_y, viewport_w, viewport_h);
    pose.hand_right        = worldToScreenPoint(camera, character.hand_right, ground_y, viewport_w, viewport_h);
    pose.hand_left_target  = worldToScreenPoint(camera, character.hand_left_target, ground_y, viewport_w, viewport_h);
    pose.hand_right_target = worldToScreenPoint(camera, character.hand_right_target, ground_y, viewport_w, viewport_h);
    pose.knee_left         = worldToScreenPoint(camera, character.knee_left, ground_y, viewport_w, viewport_h);
    pose.knee_right        = worldToScreenPoint(camera, character.knee_right, ground_y, viewport_w, viewport_h);
    pose.foot_left         = worldToScreenPoint(camera, character.foot_left.pos, ground_y, viewport_w, viewport_h);
    pose.foot_right        = worldToScreenPoint(camera, character.foot_right.pos, ground_y, viewport_w, viewport_h);

    const SDL_FPoint reach_s = worldToScreenPoint(
        camera, {character.pelvis.x + 2.0 * L, character.pelvis.y},
        ground_y, viewport_w, viewport_h);
    pose.reach_radius = std::abs(reach_s.x - pose.pelvis.x);

    const SDL_FPoint head_r_s = worldToScreenPoint(
        camera, {character.head_center.x + character.head_radius, character.head_center.y},
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
                                         const ScreenSpacePose& pose,
                                         float debug_scale) const
{
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
    drawDebugLine(renderer, pose.torso_top.x, pose.torso_top.y, pose.elbow_left.x, pose.elbow_left.y, debug_scale);
    drawDebugLine(renderer, pose.elbow_left.x, pose.elbow_left.y, pose.hand_left.x, pose.hand_left.y, debug_scale);
    drawDebugLine(renderer, pose.torso_top.x, pose.torso_top.y, pose.elbow_right.x, pose.elbow_right.y, debug_scale);
    drawDebugLine(renderer, pose.elbow_right.x, pose.elbow_right.y, pose.hand_right.x, pose.hand_right.y, debug_scale);
    drawDebugLine(renderer, pose.pelvis.x, pose.pelvis.y, pose.torso_center.x, pose.torso_center.y, debug_scale);
    drawDebugLine(renderer, pose.torso_center.x, pose.torso_center.y, pose.torso_top.x, pose.torso_top.y, debug_scale);
    drawDebugLine(renderer, pose.torso_top.x, pose.torso_top.y, pose.neck.x, pose.neck.y, debug_scale);

    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawFilledCircle(renderer, pose.pelvis.x, pose.pelvis.y, scaled(4.f, debug_scale));
    drawFilledCircle(renderer, pose.torso_center.x, pose.torso_center.y, scaled(4.f, debug_scale));
    drawFilledCircle(renderer, pose.torso_top.x, pose.torso_top.y, scaled(4.f, debug_scale));
    drawFilledCircle(renderer, pose.elbow_left.x, pose.elbow_left.y, scaled(3.f, debug_scale));
    drawFilledCircle(renderer, pose.elbow_right.x, pose.elbow_right.y, scaled(3.f, debug_scale));

    drawPinnableHand(renderer, pose.hand_left,  character.hand_left_pinned,  debug_scale);
    drawPinnableHand(renderer, pose.hand_right, character.hand_right_pinned, debug_scale);

    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawCircleOutline(renderer, pose.head.x, pose.head.y, pose.head_radius);

    if (!character.feet_initialized) return;

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
    drawDebugLine(renderer, pose.pelvis.x, pose.pelvis.y, pose.knee_left.x, pose.knee_left.y, debug_scale);
    drawDebugLine(renderer, pose.knee_left.x, pose.knee_left.y, pose.foot_left.x, pose.foot_left.y, debug_scale);
    drawDebugLine(renderer, pose.pelvis.x, pose.pelvis.y, pose.knee_right.x, pose.knee_right.y, debug_scale);
    drawDebugLine(renderer, pose.knee_right.x, pose.knee_right.y, pose.foot_right.x, pose.foot_right.y, debug_scale);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);
    drawFilledCircle(renderer, pose.knee_left.x, pose.knee_left.y, scaled(3.f, debug_scale));
    drawFilledCircle(renderer, pose.knee_right.x, pose.knee_right.y, scaled(3.f, debug_scale));
}

void CharacterRenderer::renderVisualLayer(SDL_Renderer* renderer,
                                          const Camera2D& camera,
                                          const CharacterState& character,
                                          const CharacterConfig& charConfig,
                                          const SplineRenderConfig& splineConfig,
                                          double ground_y,
                                          int viewport_w,
                                          int viewport_h) const
{
    renderSplinePass(renderer, camera, character, charConfig, splineConfig,
                     ground_y, viewport_w, viewport_h);
}

void CharacterRenderer::renderLegacyDebugLayer(SDL_Renderer* renderer,
                                               const Camera2D& camera,
                                               const CharacterState& character,
                                               const CharacterConfig& charConfig,
                                               const CMConfig& cmConfig,
                                               const Terrain& terrain,
                                               const ScreenSpacePose& pose,
                                               double ground_y,
                                               int viewport_w,
                                               int viewport_h,
                                               float debug_scale) const
{
    renderDebugMarkersBeforeBody(renderer, camera, character, charConfig, cmConfig, terrain, pose,
                                 ground_y, viewport_w, viewport_h, debug_scale);
    renderLegacyBody(renderer, character, pose, debug_scale);
    renderDebugMarkersAfterBody(renderer, camera, character, pose,
                                ground_y, viewport_w, viewport_h, debug_scale);
}

void CharacterRenderer::renderDebugMarkersBeforeBody(SDL_Renderer* renderer,
                                                     const Camera2D& camera,
                                                     const CharacterState& character,
                                                     const CharacterConfig& charConfig,
                                                     const CMConfig& cmConfig,
                                                     const Terrain& terrain,
                                                     const ScreenSpacePose& pose,
                                                     double ground_y,
                                                     int viewport_w,
                                                     int viewport_h,
                                                     float debug_scale) const
{
    if (cmConfig.show_support_line
        && character.feet_initialized
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
            drawDebugLine(renderer, prev.x, prev.y, cur.x, cur.y, debug_scale);
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
                                                    int viewport_h,
                                                    float debug_scale) const
{
    drawHandTarget(renderer, character.hand_left_pinned, pose.hand_left_target, debug_scale);
    drawHandTarget(renderer, character.hand_right_pinned, pose.hand_right_target, debug_scale);

    if (character.feet_initialized) {
        drawFootMarker(renderer, camera, character.foot_left, pose.foot_left,
                       ground_y, viewport_w, viewport_h, debug_scale);
        drawFootMarker(renderer, camera, character.foot_right, pose.foot_right,
                       ground_y, viewport_w, viewport_h, debug_scale);
    }

    SDL_SetRenderDrawColor(renderer, 0, 170, 255, 255);
    drawFilledCircle(renderer, pose.cm.x, pose.cm.y, scaled(CM_RADIUS, debug_scale));
}

void CharacterRenderer::render(SDL_Renderer*         renderer,
                               const Camera2D&       camera,
                               const CMState&        cm,
                               const CharacterState& character,
                               const CharacterConfig& charConfig,
                               const SplineRenderConfig& splineConfig,
                               const CharacterReconstructionConfig& /*reconstruction*/,
                               const CMConfig&       cmConfig,
                               bool                  spline_only,
                               bool                  legacy_debug_markers,
                               const Terrain&        terrain,
                               double                ground_y,
                               int                   viewport_w,
                               int                   viewport_h,
                               float                 debug_scale) const
{
    const ScreenSpacePose pose = computeScreenSpacePose(camera, cm, character, charConfig,
                                                        ground_y, viewport_w, viewport_h);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (spline_only) {
        renderDebugMarkersBeforeBody(renderer, camera, character, charConfig, cmConfig, terrain, pose,
                                     ground_y, viewport_w, viewport_h, debug_scale);
        renderVisualLayer(renderer, camera, character, charConfig, splineConfig,
                          ground_y, viewport_w, viewport_h);
        return;
    }

    if (splineConfig.enabled && splineConfig.draw_under_legacy)
        renderVisualLayer(renderer, camera, character, charConfig, splineConfig,
                          ground_y, viewport_w, viewport_h);

    if (legacy_debug_markers) {
        renderLegacyDebugLayer(renderer, camera, character, charConfig, cmConfig, terrain, pose,
                               ground_y, viewport_w, viewport_h, debug_scale);
    } else {
        renderLegacyBody(renderer, character, pose, debug_scale);
    }

    if (splineConfig.enabled && !splineConfig.draw_under_legacy)
        renderVisualLayer(renderer, camera, character, charConfig, splineConfig,
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
    path.cubicTo({c.x + r, c.y + k}, {c.x + k, c.y + r}, {c.x,     c.y + r});
    path.cubicTo({c.x - k, c.y + r}, {c.x - r, c.y + k}, {c.x - r, c.y});
    path.cubicTo({c.x - r, c.y - k}, {c.x - k, c.y - r}, {c.x,     c.y - r});
    path.cubicTo({c.x + k, c.y - r}, {c.x + r, c.y - k}, {c.x + r, c.y});
    path.closePath();

    renderFlattenedPath(renderer, m_strokeRenderer, camera, path, 0,
                        splineConfig, ground_y, viewport_w, viewport_h);
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
    const Vec2 neck = character.head_center - neck_dir_world * character.head_radius;

    const Vec2 handle0 = character.pelvis + (character.torso_center - character.pelvis) * 0.85;
    const Vec2 handle1 = neck - (neck - character.torso_top) * 0.85;

    StrokePath path;
    path.moveTo(character.pelvis);
    path.cubicTo(handle0, handle1, neck);

    renderFlattenedPath(renderer, m_strokeRenderer, camera, path, 1,
                        splineConfig, ground_y, viewport_w, viewport_h);
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
    const Vec2 handle0 = shoulder + (elbow - shoulder) * (2.0 / 3.0);
    const Vec2 handle1 = hand - (hand - elbow) * (2.0 / 3.0);

    StrokePath path;
    path.moveTo(shoulder);
    path.cubicTo(handle0, handle1, hand);

    renderFlattenedPath(renderer, m_strokeRenderer, camera, path, depth_index,
                        splineConfig, ground_y, viewport_w, viewport_h);
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
    const Vec2 handle0 = pelvis + (knee - pelvis) * (2.0 / 3.0);
    const Vec2 handle1 = foot - (foot - knee) * (2.0 / 3.0);

    StrokePath path;
    path.moveTo(pelvis);
    path.cubicTo(handle0, handle1, foot);

    renderFlattenedPath(renderer, m_strokeRenderer, camera, path, depth_index,
                        splineConfig, ground_y, viewport_w, viewport_h);
}
