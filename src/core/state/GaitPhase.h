#pragma once

/// @brief Phase du cycle de support du pas.
/// Ne représente que le cycle de support — ne pas y ajouter des états
/// propres à d'autres modes (Jump, Fall, etc.).
enum class GaitPhase {
    None,
    DoubleSupport,
    LeftSupport,
    RightSupport,
    Flight
};
