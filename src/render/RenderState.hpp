#pragma once

#include "core/math/Vec2.hpp"
#include "platform/PlatformRuntime.hpp"

#include <cstdint>
#include <vector>

namespace bobtricks {

/**
 * \brief Couleur RGBA simple pour le rendu.
 */
struct RgbaColor {
    std::uint8_t red {0};
    std::uint8_t green {0};
    std::uint8_t blue {0};
    std::uint8_t alpha {255};
};

/**
 * \brief Segment 2D a dessiner.
 */
struct LineSegment {
    Vec2 start {};
    Vec2 end {};
};

/**
 * \brief Point articulaire visible.
 */
struct JointPoint {
    Vec2 position {};
    double radius {0.0};
};

/**
 * \brief Etat de rendu minimal consomme par le renderer.
 */
struct RenderState {
    WindowSize viewport {960, 540};
    RgbaColor clearColor {20, 20, 20, 255};
    RgbaColor skeletonColor {44, 44, 44, 255};
    RgbaColor jointColor {70, 70, 70, 255};
    std::vector<LineSegment> skeletonSegments {};
    std::vector<JointPoint> jointPoints {};
    Vec2 headCenter {};
    double headRadius {0.0};
};

}  // namespace bobtricks
