#pragma once

#include "core/math/Vec2.hpp"
#include "core/state/CMState.hpp"
#include "core/state/FacingDirection.hpp"
#include "core/state/GaitPhase.hpp"
#include "core/state/LocomotionMode.hpp"
#include "core/state/ProceduralPoseState.hpp"
#include "core/state/StickmanGeometry.hpp"
#include "core/state/StickmanNodePositions.hpp"
#include "core/state/SupportState.hpp"
#include "core/state/SupportSide.hpp"

namespace bobtricks {

/**
 * \brief Snapshot autoritaire minimal du personnage.
 */
struct CharacterState {
    LocomotionMode mode {LocomotionMode::Stand};
    FacingDirection facingDirection {FacingDirection::Right};
    GaitPhase gaitPhase {GaitPhase::DoubleSupport};
    SupportSide supportSide {SupportSide::Both};
    StickmanGeometry geometry {};
    StickmanNodePositions nodes {};
    CMState cm {};
    SupportState support {};
    ProceduralPoseState proceduralPose {};
    Vec2 rootPosition {};
    double simulationTimeS {0.0};
};

}  // namespace bobtricks
