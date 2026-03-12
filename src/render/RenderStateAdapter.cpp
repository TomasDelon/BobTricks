#include "render/RenderStateAdapter.hpp"

namespace bobtricks {

Vec2 RenderStateAdapter::toViewportPoint(const Vec2& worldPoint, WindowSize viewport) {
    constexpr double pixelsPerMeter = 180.0;

    return {
        viewport.width * 0.5 + worldPoint.x * pixelsPerMeter,
        viewport.height * 0.85 - worldPoint.y * pixelsPerMeter
    };
}

RenderState RenderStateAdapter::build(const CharacterState& characterState, WindowSize viewport) const {
    RenderState renderState;
    renderState.viewport = viewport;

    switch (characterState.mode) {
    case LocomotionMode::Walk:
        renderState.clearColor = {156, 164, 176, 255};
        break;
    case LocomotionMode::Run:
        renderState.clearColor = {178, 156, 156, 255};
        break;
    default:
        renderState.clearColor = {190, 188, 182, 255};
        break;
    }

    renderState.skeletonColor = {38, 42, 48, 255};
    renderState.jointColor = {92, 98, 108, 255};

    const auto& nodes = characterState.nodes;
    const auto screenHeadTop = toViewportPoint(nodes.headTop, viewport);
    const auto screenTorsoTop = toViewportPoint(nodes.torsoTop, viewport);
    const auto screenTorsoCenter = toViewportPoint(nodes.torsoCenter, viewport);
    const auto screenTorsoBottom = toViewportPoint(nodes.torsoBottom, viewport);
    const auto screenElbowLeft = toViewportPoint(nodes.elbowLeft, viewport);
    const auto screenWristLeft = toViewportPoint(nodes.wristLeft, viewport);
    const auto screenElbowRight = toViewportPoint(nodes.elbowRight, viewport);
    const auto screenWristRight = toViewportPoint(nodes.wristRight, viewport);
    const auto screenKneeLeft = toViewportPoint(nodes.kneeLeft, viewport);
    const auto screenAnkleLeft = toViewportPoint(nodes.ankleLeft, viewport);
    const auto screenKneeRight = toViewportPoint(nodes.kneeRight, viewport);
    const auto screenAnkleRight = toViewportPoint(nodes.ankleRight, viewport);

    renderState.skeletonSegments = {
        {screenTorsoTop, screenTorsoCenter},
        {screenTorsoCenter, screenTorsoBottom},
        {screenTorsoTop, screenElbowLeft},
        {screenElbowLeft, screenWristLeft},
        {screenTorsoTop, screenElbowRight},
        {screenElbowRight, screenWristRight},
        {screenTorsoBottom, screenKneeLeft},
        {screenKneeLeft, screenAnkleLeft},
        {screenTorsoBottom, screenKneeRight},
        {screenKneeRight, screenAnkleRight}
    };

    renderState.jointPoints = {
        {screenTorsoTop, 5.0},
        {screenTorsoCenter, 4.0},
        {screenTorsoBottom, 5.0},
        {screenElbowLeft, 4.0},
        {screenWristLeft, 4.0},
        {screenElbowRight, 4.0},
        {screenWristRight, 4.0},
        {screenKneeLeft, 4.0},
        {screenAnkleLeft, 4.0},
        {screenKneeRight, 4.0},
        {screenAnkleRight, 4.0}
    };

    renderState.headCenter = {
        0.5 * (screenHeadTop.x + screenTorsoTop.x),
        0.5 * (screenHeadTop.y + screenTorsoTop.y)
    };
    renderState.headRadius = characterState.geometry.headRadius * 180.0;

    return renderState;
}

}  // namespace bobtricks
