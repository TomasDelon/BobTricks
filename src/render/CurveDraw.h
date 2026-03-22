#pragma once

// Bridge between curves_lab and SDL renderer.
// Zero dependencies on curves_lab types — works via any callable eval : double -> Vec2
// and any callable toScreen : Vec2 -> SDL_FPoint.
//
// Usage:
//   auto ts = [&](Vec2 v){ return camera.worldToScreen(...); };
//   render::strokeCurve(renderer, 12, [&](double t){ return bez.evalByBernstein(t); }, ts);
//   render::strokeCurve(renderer, 16, [&](double t){ return seg.eval(t); }, ts);

#include <SDL2/SDL.h>
#include "core/math/Vec2.h"

namespace render {

// Tessellate and draw any parametric curve as a polyline.
//   n_segs  : number of line segments (12 is plenty for a limb).
//   eval    : callable (double t) -> Vec2,  t in [0, 1].
//   toScreen: callable (Vec2)    -> SDL_FPoint.
template<typename EvalFn, typename ToScreenFn>
void strokeCurve(SDL_Renderer* renderer,
                 int           n_segs,
                 EvalFn        eval,
                 ToScreenFn    toScreen)
{
    if (n_segs < 1) return;
    SDL_FPoint prev = toScreen(eval(0.0));
    for (int i = 1; i <= n_segs; ++i) {
        const double   t    = static_cast<double>(i) / static_cast<double>(n_segs);
        SDL_FPoint     curr = toScreen(eval(t));
        SDL_RenderDrawLineF(renderer, prev.x, prev.y, curr.x, curr.y);
        prev = curr;
    }
}

} // namespace render
