#pragma once

namespace bobtricks {

/**
 * \brief Parametres geometriques nominaux du stickman.
 */
struct StickmanGeometry {
    double totalHeight {1.80};
    double totalMassKg {60.0};
    double limbRestLength {totalHeight / 5.0};
    double torsoSegmentLength {limbRestLength};
    double upperArmLength {limbRestLength};
    double lowerArmLength {limbRestLength};
    double upperLegLength {limbRestLength};
    double lowerLegLength {limbRestLength};
    double headRadius {0.5 * limbRestLength};
};

}  // namespace bobtricks
