#include "SimulationCore.h"

SimulationCore::SimulationCore()
    : tuning_{}
    , loco_{}
    , animator_{}
    , state_{}
{}

void SimulationCore::step(double dt, const IntentRequest& intent)
{
    loco_.update(dt, intent, tuning_);

    animator_.update(
        loco_.getCMState(),
        loco_.getLeftFootPos(),
        loco_.getRightFootPos(),
        loco_.getPoseState(),
        tuning_
    );

    // Publier le snapshot autoritatif
    state_.mode           = loco_.getPoseState().mode;
    state_.gait_phase     = loco_.getPoseState().gait_phase;
    state_.support_side   = loco_.getPoseState().support_side;
    state_.cm.procedural  = loco_.getCMState();
    state_.procedural_pose = loco_.getPoseState();
    state_.node_positions = animator_.getNodePositions();
}

const CharacterState& SimulationCore::getState() const
{
    return state_;
}
