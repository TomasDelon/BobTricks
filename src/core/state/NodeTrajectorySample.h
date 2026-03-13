#pragma once

#include "NodeId.h"
#include "../math/Vec2.h"

/// @brief Échantillon de trajectoire d'un nœud articulaire.
struct NodeTrajectorySample {
    double sim_time = 0.0;
    NodeId node     = NodeId::HeadTop;
    Vec2   position = {};
};
