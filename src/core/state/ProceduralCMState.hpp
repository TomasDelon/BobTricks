#pragma once

#include "core/math/Vec2.hpp"

namespace bobtricks {

/**
 * \brief Etat cible du centre de masse procedural.
 */
struct ProceduralCMState {
    Vec2 targetPosition {};
    Vec2 targetVelocity {};
    double operatingHeight {0.0};
    double pelvisOffsetTarget {0.0};
    double trunkLeanTarget {0.0};
};

}  // namespace bobtricks
