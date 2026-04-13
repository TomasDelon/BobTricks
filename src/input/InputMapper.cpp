#include "input/InputMapper.hpp"

namespace bobtricks {

IntentRequest InputMapper::map(const PlatformEvents& events, const IntentRequest& previousIntent) const {
    IntentRequest nextIntent = previousIntent;

    if (events.requestedMode.has_value()) {
        nextIntent.requestedMode = *events.requestedMode;
    }

    nextIntent.resetRequested = events.resetRequested;
    return nextIntent;
}

}  // namespace bobtricks
