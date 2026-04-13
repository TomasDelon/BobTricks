#pragma once

#include "core/math/Vec2.hpp"

namespace bobtricks {

/**
 * \brief Positions monde des noeuds visibles du stickman.
 */
struct StickmanNodePositions {
    Vec2 headTop {};
    Vec2 torsoTop {};
    Vec2 torsoCenter {};
    Vec2 torsoBottom {};
    Vec2 elbowLeft {};
    Vec2 wristLeft {};
    Vec2 elbowRight {};
    Vec2 wristRight {};
    Vec2 kneeLeft {};
    Vec2 ankleLeft {};
    Vec2 kneeRight {};
    Vec2 ankleRight {};
};

}  // namespace bobtricks
