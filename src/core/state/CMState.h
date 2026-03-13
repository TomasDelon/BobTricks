#pragma once

#include "../math/Vec2.h"

/// @brief État du centre de masse dans le régime procédural (cible contrôlée).
struct ProceduralCMState {
    Vec2   target_position    = {};
    Vec2   target_velocity    = {};
    double operating_height   = 0.0;
    double pelvis_offset_target = 0.0;
    double trunk_lean_target  = 0.0;  ///< en radians, positif vers l'avant
};

/// @brief État du centre de masse émergent du régime physique (lecture Box2D).
/// Invalide au mi-parcours — le champ @c valid doit être vérifié avant usage.
struct PhysicalCMState {
    Vec2   position                  = {};
    Vec2   velocity                  = {};
    Vec2   acceleration              = {};
    double operating_height_estimate = 0.0;
    bool   valid                     = false;
};

/// @brief Conteneur unifié des deux branches CM.
/// La séparation est obligatoire dès maintenant même si la branche physique est inactive.
struct CMState {
    ProceduralCMState procedural;
    PhysicalCMState   physical;
};
