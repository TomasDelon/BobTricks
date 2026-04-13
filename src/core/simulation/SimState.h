#pragma once

#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "core/math/Vec2.h"

#include <cstdint>

/** @brief Instantané complet de l'état de simulation à un instant donné. */
struct SimState {
    CMState        cm;
    CharacterState character;
    double         sim_time = 0.0;

    // XCoM (ξ = x_cm + α·v/ω₀) — computed in SimulationCore::step, cached here
    // so renderers don't need to recompute with potentially stale config values.
    double xi            = 0.0;
    double xi_target_x   = 0.0;   // ξ + facing × margin × L (foot landing target)
    bool   xi_trigger    = false;  // ξ crossed the front foot (step needed)
};

/** @brief Conditions initiales utilisées par `reset()` et les scénarios headless. */
struct ScenarioInit {
    Vec2     cm_pos       = {0.0, 0.0};
    Vec2     cm_vel       = {0.0, 0.0};
    uint32_t terrain_seed = 0;         // 0 = keep current seed
};
