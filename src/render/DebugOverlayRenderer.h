#pragma once

#include <SDL2/SDL.h>
#include <deque>
#include "render/Camera2D.h"
#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "core/character/BalanceState.h"
#include "core/character/StepPlan.h"
#include "core/terrain/Terrain.h"
#include "config/AppConfig.h"
#include "core/character/TrailPoint.h"

class DebugOverlayRenderer
{
public:
    // Trail + smoothed ground ref segment + support interval — drawn before the character.
    void renderBackground(SDL_Renderer*                  renderer,
                          const Camera2D&                camera,
                          const CMState&                 cm,
                          const Terrain&                 terrain,
                          const CharacterConfig&         charConfig,
                          const CMConfig&                cmConfig,
                          const std::deque<TrailPoint>&  trail,
                          double                         sim_time,
                          double                         ground_y,
                          int                            viewport_w,
                          int                            viewport_h,
                          const SupportState&            support) const;

    // Projection line/dot, nominal tick, vectors, drag preview — drawn after character.
    // ref_h: smoothed ground reference height (pre-computed by caller).
    void renderForeground(SDL_Renderer*          renderer,
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
                          int                    viewport_h) const;

private:
    static void drawFilledCircle   (SDL_Renderer* r, float cx, float cy, float radius);
    static void drawArrowHead      (SDL_Renderer* r, float fx, float fy, float tx, float ty, float size);
    static void drawComponentArrow (SDL_Renderer* r, float fx, float fy, float tx, float ty, Uint8 r_, Uint8 g, Uint8 b);
    static void drawSupportInterval(SDL_Renderer* r, const Camera2D& camera,
                                    const SupportState& support,
                                    double ground_y, int vw, int vh);
};
