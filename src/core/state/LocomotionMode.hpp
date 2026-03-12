#pragma once

namespace bobtricks {

/**
 * \brief Modes de locomotion prevus par l'architecture finale.
 */
enum class LocomotionMode {
    Stand,
    Walk,
    Run,
    Crouch,
    Jump,
    Land,
    Fall,
    Recovery,
    GetUp
};

}  // namespace bobtricks
