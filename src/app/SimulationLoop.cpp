#include "app/SimulationLoop.hpp"

#include <algorithm>

namespace bobtricks {

void SimulationLoop::addFrameTime(double realDeltaSeconds, double timeScale, bool paused) {
    stepsRunThisFrame_ = 0;
    if (paused) {
        return;
    }

    const double scaledDelta = std::max(0.0, realDeltaSeconds * timeScale);
    accumulatorSeconds_ += scaledDelta;
    accumulatorSeconds_ = std::min(accumulatorSeconds_, kFixedStepSeconds * kMaxStepsPerFrame);
}

bool SimulationLoop::shouldRunStep() const {
    return stepsRunThisFrame_ < kMaxStepsPerFrame && accumulatorSeconds_ >= kFixedStepSeconds;
}

void SimulationLoop::consumeStep() {
    accumulatorSeconds_ -= kFixedStepSeconds;
    ++stepsRunThisFrame_;
}

}  // namespace bobtricks
