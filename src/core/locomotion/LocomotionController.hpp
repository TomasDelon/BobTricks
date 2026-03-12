#pragma once

#include "core/state/CharacterState.hpp"
#include "core/state/TuningParams.hpp"
#include "input/IntentRequest.hpp"

namespace bobtricks {

/**
 * \brief Controleur minimal des transitions de locomotion.
 */
class LocomotionController {
public:
    /**
     * \brief Applique l'intention courante au snapshot du personnage.
     */
    void applyIntent(const IntentRequest& intent, const TuningParams& tuningParams, CharacterState& characterState) const;

    /**
     * \brief Avance les temporisateurs et les phases du cycle.
     */
    void advance(double dtSeconds, CharacterState& characterState) const;
};

}  // namespace bobtricks
