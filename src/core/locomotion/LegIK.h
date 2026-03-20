#pragma once

#include "core/math/Vec2.h"

// Analytic 2-segment leg IK.
// Each segment has equal length L (thigh = shin = L).
// Full leg reach = 2L.
//
// Given pelvis P, desired foot F, segment length L, and facing direction (+1 or -1),
// returns knee K and the effective foot F_eff (clamped to reach if needed),
// such that |P-K| == |K-F_eff| == L and the knee bends forward (in the facing direction).
//
// Always use result.foot_eff when drawing the shin segment — never the raw foot position —
// to keep the rendered chain geometrically consistent with the solver.
struct LegIKResult {
    Vec2 knee;
    Vec2 foot_eff;  // equals F if reachable, clamped otherwise
};

LegIKResult computeKnee(Vec2 P, Vec2 F, double L, double facing);
