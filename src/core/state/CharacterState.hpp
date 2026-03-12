#pragma once

#include "core/math/Vec2.hpp"
#include "core/state/CMState.hpp"
#include "core/state/GaitPhase.hpp"
#include "core/state/LocomotionMode.hpp"
#include "core/state/ProceduralPoseState.hpp"
#include "core/state/SupportState.hpp"
#include "core/state/SupportSide.hpp"

namespace bobtricks {

/**
 * \brief Snapshot autoritaire minimal du personnage.
 */
struct CharacterState {
    LocomotionMode mode {LocomotionMode::Stand};
    GaitPhase gaitPhase {GaitPhase::DoubleSupport};
    SupportSide supportSide {SupportSide::Both};
    CMState cm {};
    SupportState support {};
    ProceduralPoseState proceduralPose {};
    Vec2 rootPosition {};
    double simulationTimeS {0.0};
};

}  // namespace bobtricks
