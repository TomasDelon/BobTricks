#pragma once

#include "core/math/Vec2.hpp"

namespace bobtricks {

/**
 * \brief Etat emergent du centre de masse physique.
 */
struct PhysicalCMState {
    Vec2 position {};
    Vec2 velocity {};
    Vec2 acceleration {};
    double operatingHeightEstimate {0.0};
    bool valid {false};
};

}  // namespace bobtricks
