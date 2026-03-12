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

struct PhaseDecision {
    GaitPhase gaitPhase;
    SupportSide supportSide;
};

PhaseDecision walkPhaseForCycle(double cyclePhase) {
    if (cyclePhase < 0.10) {
        return {GaitPhase::DoubleSupport, SupportSide::Both};
    }
    if (cyclePhase < 0.50) {
        return {GaitPhase::LeftSupport, SupportSide::Left};
    }
    if (cyclePhase < 0.60) {
        return {GaitPhase::DoubleSupport, SupportSide::Both};
    }
    return {GaitPhase::RightSupport, SupportSide::Right};
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
    characterState.proceduralPose.facingDirection = characterState.facingDirection;
    characterState.proceduralPose.cycleDurationS = cycleDurationForMode(intent.requestedMode, tuningParams);
}

void LocomotionController::advance(double dtSeconds, CharacterState& characterState) const {
    characterState.simulationTimeS += dtSeconds;
    characterState.proceduralPose.modeTime += dtSeconds;
    characterState.proceduralPose.gaitPhaseTime += dtSeconds;

    const auto mode = characterState.mode;
    if (mode == LocomotionMode::Stand) {
        characterState.gaitPhase = GaitPhase::None;
        characterState.supportSide = SupportSide::Both;
        characterState.proceduralPose.gaitPhase = GaitPhase::None;
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
        const PhaseDecision walkPhase = walkPhaseForCycle(cyclePhase);
        characterState.gaitPhase = walkPhase.gaitPhase;
        characterState.supportSide = walkPhase.supportSide;
    }

    characterState.proceduralPose.gaitPhase = characterState.gaitPhase;
    characterState.proceduralPose.supportSide = characterState.supportSide;
}

}  // namespace bobtricks
