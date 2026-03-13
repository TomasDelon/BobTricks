#pragma once

#include "SupportSide.h"

/// @brief État de support au sol du personnage.
struct SupportState {
    SupportSide active_side    = SupportSide::Both;
    bool        left_grounded  = true;
    bool        right_grounded = true;
    bool        support_valid  = true;
};
