#pragma once

#include "core/character/BalanceState.h"
#include "core/character/CMState.h"
#include "core/character/SupportState.h"
#include "config/AppConfig.h"

// Computes XCoM / Capture Point metrics (Hof 2008).
//
// ROADMAP §3.1:
//   h_ref  = y_CM_standing_target - support.ground_center()
//   omega0 = sqrt(g / h_ref)
//   xi     = cm.position.x + cm.velocity.x / omega0
//   mos    = min(xi - support.x_left, support.x_right - xi)
//
// h_ref is derived from the geometric standing target (not instantaneous CM height)
// to keep omega0 stable during walking bobbing.
// Guard: if h_ref <= 0 (degenerate), returns zero-state.
BalanceState computeBalanceState(const CMState&        cm,
                                 const SupportState&   support,
                                 const CharacterConfig& char_cfg,
                                 const PhysicsConfig&   phys_cfg);
