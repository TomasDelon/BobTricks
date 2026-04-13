#include "core/simulation/SimulationCore.hpp"

namespace bobtricks {

void SimulationCore::initialize() {
    characterState_ = {};
    characterState_.mode = LocomotionMode::Stand;
    characterState_.facingDirection = FacingDirection::Right;
    characterState_.gaitPhase = GaitPhase::None;
    characterState_.supportSide = SupportSide::Both;
    characterState_.geometry = {};
    characterState_.nodes = {};
    characterState_.proceduralPose.mode = LocomotionMode::Stand;
    characterState_.proceduralPose.facingDirection = FacingDirection::Right;
    characterState_.proceduralPose.gaitPhase = GaitPhase::None;
    characterState_.proceduralPose.supportSide = SupportSide::Both;
    characterState_.support.activeSide = SupportSide::Both;
    characterState_.support.leftGrounded = true;
    characterState_.support.rightGrounded = true;
}

void SimulationCore::setIntent(const IntentRequest& intent) {
    intent_ = intent;
}

void SimulationCore::step(double dtSeconds) {
    locomotionController_.applyIntent(intent_, tuningParams_, characterState_);
    locomotionController_.advance(dtSeconds, characterState_);
    proceduralAnimator_.update(dtSeconds, characterState_);
    intent_.resetRequested = false;
}

const CharacterState& SimulationCore::getCharacterState() const {
    return characterState_;
}

}  // namespace bobtricks
