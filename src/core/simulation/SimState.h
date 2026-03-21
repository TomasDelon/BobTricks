#pragma once

#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "core/math/Vec2.h"

#include <cstdint>

// Complete snapshot of simulation state at one point in time.
// Read-only outside SimulationCore — renderers and telemetry observe this.
struct SimState {
    CMState        cm;
    CharacterState character;
    double         sim_time = 0.0;
};

// Initial conditions for reset() / headless scenario start.
// Feet are always bootstrapped on the first step() after reset().
struct ScenarioInit {
    Vec2     cm_pos       = {0.0, 0.0};
    Vec2     cm_vel       = {0.0, 0.0};
    uint32_t terrain_seed = 0;         // 0 = keep current seed
};
