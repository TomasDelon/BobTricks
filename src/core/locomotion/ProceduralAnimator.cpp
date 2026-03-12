#include "core/locomotion/ProceduralAnimator.hpp"

#include <cmath>

namespace bobtricks {

namespace {

Vec2 normalize(const Vec2& value) {
    const double magnitude = std::sqrt(value.x * value.x + value.y * value.y);
    if (magnitude <= 1e-9) {
        return {0.0, 0.0};
    }
    return {value.x / magnitude, value.y / magnitude};
}

Vec2 addScaled(const Vec2& origin, const Vec2& direction, double distance) {
    return {
        origin.x + direction.x * distance,
        origin.y + direction.y * distance
    };
}

}

void ProceduralAnimator::update(double dtSeconds, CharacterState& characterState) const {
    (void)dtSeconds;

    const auto& geometry = characterState.geometry;
    const Vec2 worldRoot = characterState.rootPosition;
    const double legReach = geometry.upperLegLength + geometry.lowerLegLength;
    const double stanceHalfWidth = 0.33 * geometry.lowerLegLength;
    const double pelvisHeight = legReach - 0.08 * geometry.lowerLegLength;
    const double kneeHeight = pelvisHeight * 0.52;
    const double kneeInset = 0.28 * stanceHalfWidth;
    const double torsoBottomHeight = pelvisHeight;
    const double torsoCenterHeight = torsoBottomHeight + geometry.torsoSegmentLength;
    const double torsoTopHeight = torsoCenterHeight + geometry.torsoSegmentLength;
    const double headTopHeight = torsoTopHeight + 2.0 * geometry.headRadius;

    const Vec2 ankleLeft = worldRoot + Vec2{-stanceHalfWidth, 0.0};
    const Vec2 ankleRight = worldRoot + Vec2{stanceHalfWidth, 0.0};
    const Vec2 kneeLeft = worldRoot + Vec2{-(stanceHalfWidth - kneeInset), kneeHeight};
    const Vec2 kneeRight = worldRoot + Vec2{(stanceHalfWidth - kneeInset), kneeHeight};
    const Vec2 torsoBottom = worldRoot + Vec2{0.0, torsoBottomHeight};
    const Vec2 torsoCenter = worldRoot + Vec2{0.0, torsoCenterHeight};
    const Vec2 torsoTop = worldRoot + Vec2{0.0, torsoTopHeight};
    const Vec2 headTop = worldRoot + Vec2{0.0, headTopHeight};

    const Vec2 upperArmLeftDirection = normalize({-0.42, -1.0});
    const Vec2 lowerArmLeftDirection = normalize({-0.22, -1.0});
    const Vec2 upperArmRightDirection = normalize({0.42, -1.0});
    const Vec2 lowerArmRightDirection = normalize({0.22, -1.0});

    const Vec2 elbowLeft = addScaled(torsoTop, upperArmLeftDirection, geometry.upperArmLength);
    const Vec2 wristLeft = addScaled(elbowLeft, lowerArmLeftDirection, geometry.lowerArmLength);
    const Vec2 elbowRight = addScaled(torsoTop, upperArmRightDirection, geometry.upperArmLength);
    const Vec2 wristRight = addScaled(elbowRight, lowerArmRightDirection, geometry.lowerArmLength);

    characterState.cm.procedural.targetPosition = {
        worldRoot.x,
        worldRoot.y + 0.57 * geometry.totalHeight
    };
    characterState.cm.procedural.targetVelocity = {};
    characterState.cm.procedural.operatingHeight = 0.57 * geometry.totalHeight;
    characterState.cm.procedural.pelvisOffsetTarget = 0.0;
    characterState.cm.procedural.trunkLeanTarget = 0.0;

    characterState.nodes.ankleLeft = ankleLeft;
    characterState.nodes.ankleRight = ankleRight;
    characterState.nodes.kneeLeft = kneeLeft;
    characterState.nodes.kneeRight = kneeRight;
    characterState.nodes.torsoBottom = torsoBottom;
    characterState.nodes.torsoCenter = torsoCenter;
    characterState.nodes.torsoTop = torsoTop;
    characterState.nodes.headTop = headTop;
    characterState.nodes.elbowLeft = elbowLeft;
    characterState.nodes.wristLeft = wristLeft;
    characterState.nodes.elbowRight = elbowRight;
    characterState.nodes.wristRight = wristRight;
}

}  // namespace bobtricks
