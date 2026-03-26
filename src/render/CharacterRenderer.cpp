#include "render/CharacterRenderer.h"

#include <cmath>

static constexpr float CM_RADIUS = 4.f;

void CharacterRenderer::drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius)
{
    const int ir = static_cast<int>(std::ceil(radius));
    for (int dy = -ir; dy <= ir; ++dy) {
        const float fy = static_cast<float>(dy);
        const float dx = std::sqrt(std::max(0.f, radius * radius - fy * fy));
        SDL_RenderDrawLineF(renderer, cx - dx, cy + fy, cx + dx, cy + fy);
    }
}

void CharacterRenderer::render(SDL_Renderer*         renderer,
                               const Camera2D&       camera,
                               const CMState&        cm,
                               const CharacterState& character,
                               const CMConfig&       cmConfig,
                               double                ground_y,
                               int                   viewport_w,
                               int                   viewport_h) const
{
    const SDL_FPoint cm_s     = camera.worldToScreen(cm.position.x,              cm.position.y,              ground_y, viewport_w, viewport_h);
    const SDL_FPoint pelvis_s = camera.worldToScreen(character.pelvis.x,         character.pelvis.y,         ground_y, viewport_w, viewport_h);
    const SDL_FPoint tc_s     = camera.worldToScreen(character.torso_center.x,   character.torso_center.y,   ground_y, viewport_w, viewport_h);
    const SDL_FPoint tt_s     = camera.worldToScreen(character.torso_top.x,      character.torso_top.y,      ground_y, viewport_w, viewport_h);

    // Torso spine — pelvis → torso_top (straight line skeleton).
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
    SDL_RenderDrawLineF(renderer, pelvis_s.x, pelvis_s.y, tt_s.x, tt_s.y);

    // Spine nodes
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    drawFilledCircle(renderer, pelvis_s.x, pelvis_s.y, 4.f);
    drawFilledCircle(renderer, tc_s.x,     tc_s.y,     4.f);
    drawFilledCircle(renderer, tt_s.x,     tt_s.y,     4.f);

    // CM dot
    SDL_SetRenderDrawColor(renderer, 0, 170, 255, 255);
    drawFilledCircle(renderer, cm_s.x, cm_s.y, CM_RADIUS);

    // Leg chains — pelvis → knee → foot, only after bootstrap has run.
    // foot_eff is the IK-clamped foot position (always within 2L of pelvis).
    // Both the shin segment and the foot dot use the same point — no gaps.
    // The planner should guarantee foot.pos == foot_eff; until it does,
    // foot_eff is the single source of truth for the rendered leg.
    if (character.feet_initialized) {
        const SDL_FPoint kl_s = camera.worldToScreen(
            character.knee_left.x,      character.knee_left.y,      ground_y, viewport_w, viewport_h);
        const SDL_FPoint kr_s = camera.worldToScreen(
            character.knee_right.x,     character.knee_right.y,     ground_y, viewport_w, viewport_h);
        const SDL_FPoint fl_s = camera.worldToScreen(
            character.foot_left_eff.x,  character.foot_left_eff.y,  ground_y, viewport_w, viewport_h);
        const SDL_FPoint fr_s = camera.worldToScreen(
            character.foot_right_eff.x, character.foot_right_eff.y, ground_y, viewport_w, viewport_h);

        // Leg skeleton — pelvis → knee → foot (fixed-length bones, no gaps).
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
        SDL_RenderDrawLineF(renderer, pelvis_s.x, pelvis_s.y, kl_s.x, kl_s.y);
        SDL_RenderDrawLineF(renderer, kl_s.x,     kl_s.y,     fl_s.x, fl_s.y);
        SDL_RenderDrawLineF(renderer, pelvis_s.x, pelvis_s.y, kr_s.x, kr_s.y);
        SDL_RenderDrawLineF(renderer, kr_s.x,     kr_s.y,     fr_s.x, fr_s.y);

        // Knee joints
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        drawFilledCircle(renderer, kl_s.x, kl_s.y, 3.f);
        drawFilledCircle(renderer, kr_s.x, kr_s.y, 3.f);

        // Foot dots — same position as shin endpoint, orange when planted.
        const bool hl        = cmConfig.show_planted_feet_color;
        const bool l_planted = (character.foot_left.phase  == FootPhase::Planted);
        const bool r_planted = (character.foot_right.phase == FootPhase::Planted);
        if (hl && l_planted) SDL_SetRenderDrawColor(renderer, 255, 160,  30, 255);
        else                  SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        drawFilledCircle(renderer, fl_s.x, fl_s.y, 4.f);
        if (hl && r_planted) SDL_SetRenderDrawColor(renderer, 255, 160,  30, 255);
        else                  SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        drawFilledCircle(renderer, fr_s.x, fr_s.y, 4.f);
    }
}
