#pragma once

#include "../state/CharacterState.h"
#include "../state/IntentRequest.h"
#include "../state/TuningParams.h"

/// @brief Contrôleur de locomotion procédurale (Stand / Walk / Run).
///
/// Avance le cycle de marche par timer pur (60 Hz), gère les transitions de
/// mode et calcule les cibles de CM et de pieds utilisées par ProceduralAnimator.
class LocomotionController {
public:
    LocomotionController();

    /// @brief Met à jour l'état d'un pas de simulation.
    /// @param dt      pas de temps en secondes (typiquement 1/60)
    /// @param intent  intention reçue de l'InputMapper
    /// @param tuning  paramètres ajustables courants
    void update(double dt, const IntentRequest& intent, const TuningParams& tuning);

    ProceduralPoseState getPoseState()    const;
    ProceduralCMState   getCMState()      const;
    Vec2                getLeftFootPos()  const;
    Vec2                getRightFootPos() const;

private:
    // --- Mode ---
    LocomotionMode mode_;
    LocomotionMode pending_mode_;
    bool           pending_mode_change_;

    // --- Cycle ---
    GaitPhase gait_phase_;
    double    normalized_cycle_;
    double    gait_phase_time_;
    double    mode_time_;
    double    cycle_duration_s_;

    // --- Locomotion ---
    double      desired_forward_speed_;
    SupportSide support_side_;

    // --- Pieds ---
    FootTarget active_swing_target_;
    Vec2       left_foot_pos_;
    Vec2       right_foot_pos_;

    // --- CM / tronc ---
    Vec2   pelvis_pos_;
    double trunk_lean_;

    bool initialized_;

    // --- Méthodes privées ---
    void      handleModeRequest(const IntentRequest& intent, const TuningParams& tuning);
    void      advanceCycle(double dt, const TuningParams& tuning);
    GaitPhase phaseFromCycle(double nc) const;
    void      onPhaseEnter(GaitPhase phase, const TuningParams& tuning);
    void      updateSwingFoot();
    void      updatePelvisHeight(const TuningParams& tuning);
    double    phaseLocalU() const;
    Vec2      swingFootPos(const FootTarget& target, double u) const;
    double    cycleDurationForMode(LocomotionMode m, const TuningParams& t) const;
    double    speedForMode(LocomotionMode m, const TuningParams& t) const;
    void      snapFeetToStand(const TuningParams& tuning);
};
