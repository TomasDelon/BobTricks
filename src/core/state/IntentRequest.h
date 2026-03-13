#pragma once

#include "LocomotionMode.h"

/// @brief Intention abstraite transmise par l'entrée utilisateur au cœur de simulation.
struct IntentRequest {
    LocomotionMode requested_mode         = LocomotionMode::Stand;
    bool           reset_requested        = false;
    bool           pause_toggle_requested = false;
    double         requested_time_scale   = 1.0;
};
