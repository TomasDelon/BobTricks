#pragma once

/// @brief Mode de locomotion actif du personnage.
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
