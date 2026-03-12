#include "core/locomotion/ProceduralAnimator.hpp"

namespace bobtricks {

void ProceduralAnimator::update(double dtSeconds, CharacterState& characterState) const {
    (void)dtSeconds;

    const auto& geometry = characterState.geometry;
    const double segment = geometry.limbRestLength;

    characterState.cm.procedural.targetPosition = characterState.rootPosition;
    characterState.cm.procedural.operatingHeight = 2.85 * segment;
    characterState.cm.procedural.pelvisOffsetTarget = 0.0;
    characterState.cm.procedural.trunkLeanTarget = 0.0;

    characterState.nodes.ankleLeft = {-0.35 * segment, 0.0};
    characterState.nodes.ankleRight = {0.35 * segment, 0.0};
    characterState.nodes.kneeLeft = {-0.22 * segment, 1.0 * segment};
    characterState.nodes.kneeRight = {0.22 * segment, 1.0 * segment};
    characterState.nodes.torsoBottom = {0.0, 2.0 * segment};
    characterState.nodes.torsoCenter = {0.0, 3.0 * segment};
    characterState.nodes.torsoTop = {0.0, 4.0 * segment};
    characterState.nodes.headTop = {0.0, 5.0 * segment};
    characterState.nodes.elbowLeft = {-0.75 * segment, 3.35 * segment};
    characterState.nodes.wristLeft = {-1.05 * segment, 2.45 * segment};
    characterState.nodes.elbowRight = {0.75 * segment, 3.35 * segment};
    characterState.nodes.wristRight = {1.05 * segment, 2.45 * segment};
}

}  // namespace bobtricks
