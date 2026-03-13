#pragma once

#include "LocomotionMode.h"
#include "GaitPhase.h"
#include "SupportSide.h"
#include "FootTarget.h"

/// @brief État procédural du cycle de pas et des cibles de pose.
struct ProceduralPoseState {
    LocomotionMode mode              = LocomotionMode::Stand;
    GaitPhase      gait_phase        = GaitPhase::None;
    double         gait_phase_time   = 0.0; ///< temps écoulé dans la phase courante (s)
    double         normalized_cycle  = 0.0; ///< progression dans le cycle [0, 1)
    double         mode_time         = 0.0; ///< temps total dans le mode courant (s)
    SupportSide    support_side      = SupportSide::Both;
    double         forward_speed     = 0.0; ///< vitesse d'avance souhaitée (m/s)
    double         cycle_duration_s  = 1.0; ///< durée d'un cycle complet (s)
    FootTarget     active_swing_target = {};
};
