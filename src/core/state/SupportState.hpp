#pragma once

#include "core/state/SupportSide.hpp"

namespace bobtricks {

/**
 * \brief Etat logique des appuis au sol.
 */
struct SupportState {
    SupportSide activeSide {SupportSide::Both};
    bool leftGrounded {true};
    bool rightGrounded {true};
    bool supportValid {true};
};

}  // namespace bobtricks
