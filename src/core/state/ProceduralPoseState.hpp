#pragma once

#include "core/state/FootTarget.hpp"
#include "core/state/FacingDirection.hpp"
#include "core/state/GaitPhase.hpp"
#include "core/state/LocomotionMode.hpp"
#include "core/state/SupportSide.hpp"

namespace bobtricks {

/**
 * \brief Etat procedural principal publie par le controleur de locomotion.
 */
struct ProceduralPoseState {
    LocomotionMode mode {LocomotionMode::Stand};
    FacingDirection facingDirection {FacingDirection::Right};
    GaitPhase gaitPhase {GaitPhase::DoubleSupport};
    double gaitPhaseTime {0.0};
    double normalizedCycle {0.0};
    double modeTime {0.0};
    SupportSide supportSide {SupportSide::Both};
    double forwardSpeed {0.0};
    double cycleDurationS {1.0};
    FootTarget activeSwingTarget {};
};

}  // namespace bobtricks
