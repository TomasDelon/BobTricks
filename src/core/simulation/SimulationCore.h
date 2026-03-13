#pragma once

#include "../state/CharacterState.h"
#include "../state/IntentRequest.h"
#include "../state/TuningParams.h"
#include "../locomotion/LocomotionController.h"
#include "../locomotion/ProceduralAnimator.h"

/// @brief Cœur de simulation : orchestre LocomotionController et ProceduralAnimator,
/// publie un CharacterState autoritatif à chaque pas.
class SimulationCore {
public:
    SimulationCore();

    /// @brief Avance la simulation d'un pas de temps fixe.
    /// @param dt     pas de temps en secondes (1/60)
    /// @param intent intention courante de l'utilisateur
    void step(double dt, const IntentRequest& intent);

    const CharacterState& getState() const;

private:
    TuningParams         tuning_;
    LocomotionController loco_;
    ProceduralAnimator   animator_;
    CharacterState       state_;
};
