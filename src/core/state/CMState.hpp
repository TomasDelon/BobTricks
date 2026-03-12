#pragma once

#include "core/state/PhysicalCMState.hpp"
#include "core/state/ProceduralCMState.hpp"

namespace bobtricks {

/**
 * \brief Regroupe la branche procedurale et la branche physique du centre de masse.
 */
struct CMState {
    ProceduralCMState procedural {};
    PhysicalCMState physical {};
};

}  // namespace bobtricks
