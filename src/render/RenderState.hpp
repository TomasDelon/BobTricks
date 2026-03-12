#pragma once

#include "platform/PlatformRuntime.hpp"

#include <cstdint>

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
 * \brief Etat de rendu minimal consomme par le renderer.
 */
struct RenderState {
    WindowSize viewport {960, 540};
    RgbaColor clearColor {20, 20, 20, 255};
};

}  // namespace bobtricks
