#include "render/RenderStateAdapter.hpp"

namespace bobtricks {

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

    return renderState;
}

}  // namespace bobtricks
