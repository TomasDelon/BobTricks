#include "core/locomotion/LocomotionController.hpp"

namespace bobtricks {

namespace {
double cycleDurationForMode(LocomotionMode mode, const TuningParams& tuningParams) {
    switch (mode) {
    case LocomotionMode::Run:
        return tuningParams.timing.runCycleDurationS;
    case LocomotionMode::Walk:
        return tuningParams.timing.walkCycleDurationS;
    default:
        return 1.0;
    }
}
}

void LocomotionController::applyIntent(
    const IntentRequest& intent,
    const TuningParams& tuningParams,
    CharacterState& characterState
) const {
    if (intent.resetRequested) {
        characterState.simulationTimeS = 0.0;
        characterState.proceduralPose.modeTime = 0.0;
        characterState.proceduralPose.gaitPhaseTime = 0.0;
        characterState.proceduralPose.normalizedCycle = 0.0;
    }

    characterState.mode = intent.requestedMode;
    characterState.proceduralPose.mode = intent.requestedMode;
    characterState.proceduralPose.cycleDurationS = cycleDurationForMode(intent.requestedMode, tuningParams);
}

void LocomotionController::advance(double dtSeconds, CharacterState& characterState) const {
    characterState.simulationTimeS += dtSeconds;
    characterState.proceduralPose.modeTime += dtSeconds;
    characterState.proceduralPose.gaitPhaseTime += dtSeconds;

    const auto mode = characterState.mode;
    if (mode == LocomotionMode::Stand) {
        characterState.gaitPhase = GaitPhase::DoubleSupport;
        characterState.supportSide = SupportSide::Both;
        characterState.proceduralPose.gaitPhase = GaitPhase::DoubleSupport;
        characterState.proceduralPose.supportSide = SupportSide::Both;
        characterState.proceduralPose.forwardSpeed = 0.0;
        characterState.proceduralPose.normalizedCycle = 0.0;
        characterState.proceduralPose.gaitPhaseTime = 0.0;
        return;
    }

    const double cycleDuration = characterState.proceduralPose.cycleDurationS;
    const double normalizedCycle = cycleDuration > 0.0
        ? characterState.proceduralPose.modeTime / cycleDuration
        : 0.0;
    const double cyclePhase = normalizedCycle - static_cast<long long>(normalizedCycle);

    characterState.proceduralPose.normalizedCycle = cyclePhase;
    characterState.proceduralPose.forwardSpeed = (mode == LocomotionMode::Run) ? 2.4 : 1.2;

    if (mode == LocomotionMode::Run) {
        characterState.gaitPhase = (cyclePhase < 0.5) ? GaitPhase::LeftSupport : GaitPhase::RightSupport;
        characterState.supportSide = (cyclePhase < 0.5) ? SupportSide::Left : SupportSide::Right;
    } else {
        characterState.gaitPhase = (cyclePhase < 0.5) ? GaitPhase::LeftSupport : GaitPhase::RightSupport;
        characterState.supportSide = (cyclePhase < 0.5) ? SupportSide::Left : SupportSide::Right;
    }

    characterState.proceduralPose.gaitPhase = characterState.gaitPhase;
    characterState.proceduralPose.supportSide = characterState.supportSide;
}

}  // namespace bobtricks
