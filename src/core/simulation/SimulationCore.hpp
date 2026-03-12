#pragma once

#include "core/locomotion/LocomotionController.hpp"
#include "core/locomotion/ProceduralAnimator.hpp"
#include "core/state/CharacterState.hpp"
#include "core/state/TuningParams.hpp"
#include "input/IntentRequest.hpp"

namespace bobtricks {

/**
 * \brief Coeur de simulation minimal independant de SDL.
 */
class SimulationCore {
public:
    /**
     * \brief Initialise l'etat autoritaire minimal du projet.
     */
    void initialize();

    /**
     * \brief Defini l'intention courante a consommer lors des pas fixes.
     */
    void setIntent(const IntentRequest& intent);

    /**
     * \brief Avance la simulation d'un pas fixe.
     */
    void step(double dtSeconds);

    /**
     * \brief Expose le snapshot autoritaire courant.
     */
    const CharacterState& getCharacterState() const;

private:
    IntentRequest intent_ {};
    TuningParams tuningParams_ {};
    CharacterState characterState_ {};
    LocomotionController locomotionController_ {};
    ProceduralAnimator proceduralAnimator_ {};
};

}  // namespace bobtricks
