#pragma once

#include "core/state/LocomotionMode.hpp"

namespace bobtricks {

/**
 * \brief Intention abstraite demandee par l'utilisateur ou un outil.
 */
struct IntentRequest {
    LocomotionMode requestedMode {LocomotionMode::Stand};
    bool resetRequested {false};
};

}  // namespace bobtricks
