#pragma once

#include <array>
#include "../core/state/CharacterState.h"

/// @brief Données prêtes à dessiner, produites par RenderStateAdapter.
/// Le renderer SDL ne lit que ce struct — il n'accède pas à CharacterState.
struct RenderState {
    std::array<Vec2, NODE_COUNT> nodes      = {};
    Vec2                         camera_pos = {}; ///< position monde centrée à l'écran
    LocomotionMode               mode       = LocomotionMode::Stand;
    GaitPhase                    gait_phase = GaitPhase::None;
};
