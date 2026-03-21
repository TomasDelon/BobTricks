#pragma once

#include <optional>
#include "core/math/Vec2.h"

// A single frame of player/scripted input, decoupled from SDL.
//
// SDLViewerApp builds this from keyboard state + mouse drag each frame.
// ScenarioRunner builds this from a scripted timeline.
// SimulationCore::step() takes one of these — it never touches SDL directly.
struct InputFrame {
    bool key_left  = false;   // accelerate left  (AZERTY: Q)
    bool key_right = false;   // accelerate right (AZERTY: D)
    bool jump      = false;   // one-shot jump impulse (AZERTY: SPACE)

    // If set, overrides CM velocity at the start of the step (drag-set in UI).
    std::optional<Vec2> set_velocity;
};
