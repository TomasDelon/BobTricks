#pragma once

#include "core/math/Vec2.hpp"
#include "core/state/NodeId.hpp"

namespace bobtricks {

/**
 * \brief Echantillon temporel d'une trajectoire de noeud.
 */
struct NodeTrajectorySample {
    double simTime {0.0};
    NodeId node {NodeId::Head};
    Vec2 position {};
};

}  // namespace bobtricks
