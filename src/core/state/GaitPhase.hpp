#pragma once

namespace bobtricks {

/**
 * \brief Phase du cycle d'appui.
 */
enum class GaitPhase {
    None,
    DoubleSupport,
    LeftSupport,
    RightSupport,
    Flight
};

}  // namespace bobtricks
