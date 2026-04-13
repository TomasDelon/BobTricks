#pragma once

#include "core/math/Vec2.hpp"
#include "core/state/FootSide.hpp"

namespace bobtricks {

/**
 * \brief Cible de pied pour une phase de balancement.
 */
struct FootTarget {
    FootSide foot {FootSide::Left};
    Vec2 takeoffPosition {};
    Vec2 targetPosition {};
    double liftHeight {0.0};
};

}  // namespace bobtricks
