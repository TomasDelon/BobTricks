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
    const double facingSign = (characterState.facingDirection == FacingDirection::Right) ? 1.0 : -1.0;
    const double legReach = geometry.upperLegLength + geometry.lowerLegLength;
    const double stanceHalfWidth = 0.33 * geometry.lowerLegLength;
    const double pelvisHeight = legReach - 0.08 * geometry.lowerLegLength;
    const double rearKneeHeight = pelvisHeight * 0.48;
    const double frontKneeHeight = pelvisHeight * 0.53;
    const double rearKneeX = -0.62 * stanceHalfWidth;
    const double frontKneeX = 0.78 * stanceHalfWidth;
    const double rearFootLead = 0.08 * geometry.lowerLegLength;
    const double frontFootLead = 0.06 * geometry.lowerLegLength;
    const double torsoBottomHeight = pelvisHeight;
    const double torsoCenterHeight = torsoBottomHeight + geometry.torsoSegmentLength;
    const double torsoTopHeight = torsoCenterHeight + geometry.torsoSegmentLength;
    const double headTopHeight = torsoTopHeight + 2.0 * geometry.headRadius;

    Vec2 ankleRear = worldRoot + Vec2{-stanceHalfWidth - facingSign * rearFootLead, 0.0};
    Vec2 ankleFront = worldRoot + Vec2{stanceHalfWidth + facingSign * frontFootLead, 0.0};
    Vec2 kneeRear = worldRoot + Vec2{rearKneeX, rearKneeHeight};
    Vec2 kneeFront = worldRoot + Vec2{frontKneeX, frontKneeHeight};

    if (facingSign < 0.0) {
        const Vec2 previousRearAnkle = ankleRear;
        const Vec2 previousRearKnee = kneeRear;
        ankleRear = ankleFront;
        ankleFront = previousRearAnkle;
        kneeRear = kneeFront;
        kneeFront = previousRearKnee;
    }

    const Vec2 torsoBottom = worldRoot + Vec2{0.0, torsoBottomHeight};
    const Vec2 torsoCenter = worldRoot + Vec2{0.0, torsoCenterHeight};
    const Vec2 torsoTop = worldRoot + Vec2{0.0, torsoTopHeight};
    const Vec2 headTop = worldRoot + Vec2{0.0, headTopHeight};

    const Vec2 upperArmRearDirection = normalize({-0.22 * facingSign, -1.0});
    const Vec2 lowerArmRearDirection = normalize({-0.06 * facingSign, -1.0});
    const Vec2 upperArmFrontDirection = normalize({0.22 * facingSign, -1.0});
    const Vec2 lowerArmFrontDirection = normalize({0.06 * facingSign, -1.0});

    Vec2 elbowRear = addScaled(torsoTop, upperArmRearDirection, geometry.upperArmLength);
    Vec2 wristRear = addScaled(elbowRear, lowerArmRearDirection, geometry.lowerArmLength);
    Vec2 elbowFront = addScaled(torsoTop, upperArmFrontDirection, geometry.upperArmLength);
    Vec2 wristFront = addScaled(elbowFront, lowerArmFrontDirection, geometry.lowerArmLength);

    characterState.cm.procedural.targetPosition = {
        worldRoot.x,
        worldRoot.y + 0.57 * geometry.totalHeight
    };
    characterState.cm.procedural.targetVelocity = {};
    characterState.cm.procedural.operatingHeight = 0.57 * geometry.totalHeight;
    characterState.cm.procedural.pelvisOffsetTarget = 0.0;
    characterState.cm.procedural.trunkLeanTarget = 0.0;
    characterState.proceduralPose.facingDirection = characterState.facingDirection;

    if (facingSign > 0.0) {
        characterState.nodes.ankleLeft = ankleRear;
        characterState.nodes.ankleRight = ankleFront;
        characterState.nodes.kneeLeft = kneeRear;
        characterState.nodes.kneeRight = kneeFront;
        characterState.nodes.elbowLeft = elbowRear;
        characterState.nodes.wristLeft = wristRear;
        characterState.nodes.elbowRight = elbowFront;
        characterState.nodes.wristRight = wristFront;
    } else {
        characterState.nodes.ankleLeft = ankleFront;
        characterState.nodes.ankleRight = ankleRear;
        characterState.nodes.kneeLeft = kneeFront;
        characterState.nodes.kneeRight = kneeRear;
        characterState.nodes.elbowLeft = elbowFront;
        characterState.nodes.wristLeft = wristFront;
        characterState.nodes.elbowRight = elbowRear;
        characterState.nodes.wristRight = wristRear;
    }
    characterState.nodes.torsoBottom = torsoBottom;
    characterState.nodes.torsoCenter = torsoCenter;
    characterState.nodes.torsoTop = torsoTop;
    characterState.nodes.headTop = headTop;
}

}  // namespace bobtricks
