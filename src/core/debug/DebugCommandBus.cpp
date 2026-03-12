#include "core/debug/DebugCommandBus.hpp"

#include <algorithm>

namespace bobtricks {

void DebugCommandBus::togglePause() {
    paused_ = !paused_;
}

void DebugCommandBus::speedUp() {
    timeScale_ = std::min(timeScale_ * 1.25, 4.0);
}

void DebugCommandBus::slowDown() {
    timeScale_ = std::max(timeScale_ * 0.8, 0.1);
}

double DebugCommandBus::getTimeScale() const {
    return timeScale_;
}

bool DebugCommandBus::isPaused() const {
    return paused_;
}

DebugSnapshot DebugCommandBus::createSnapshot() const {
    return {.paused = paused_, .timeScale = timeScale_};
}

}  // namespace bobtricks
