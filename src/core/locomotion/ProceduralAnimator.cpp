#include "core/locomotion/ProceduralAnimator.hpp"

namespace bobtricks {

void ProceduralAnimator::update(double dtSeconds, CharacterState& characterState) const {
    (void)dtSeconds;

    characterState.cm.procedural.targetPosition = characterState.rootPosition;
    characterState.cm.procedural.operatingHeight = 1.0;
    characterState.cm.procedural.pelvisOffsetTarget = 0.0;
    characterState.cm.procedural.trunkLeanTarget = 0.0;
}

}  // namespace bobtricks
