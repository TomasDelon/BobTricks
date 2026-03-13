#pragma once

#include <array>

#include "LocomotionMode.h"
#include "GaitPhase.h"
#include "SupportSide.h"
#include "CMState.h"
#include "SupportState.h"
#include "ProceduralPoseState.h"
#include "NodeId.h"
#include "../math/Vec2.h"

/// @brief Nombre de nœuds articulaires du personnage.
constexpr int NODE_COUNT = 12;

/// @brief Snapshot autoritatif complet du personnage, publié par SimulationCore.
struct CharacterState {
    LocomotionMode      mode            = LocomotionMode::Stand;
    GaitPhase           gait_phase      = GaitPhase::None;
    SupportSide         support_side    = SupportSide::Both;
    CMState             cm              = {};
    SupportState        support         = {};
    ProceduralPoseState procedural_pose = {};

    /// @brief Positions mondiales de chaque nœud, indexées par static_cast<int>(NodeId).
    std::array<Vec2, NODE_COUNT> node_positions = {};
};
