#pragma once

/**
 * @file TelemetryRow.h
 * @brief Ligne élémentaire de télémétrie exportée au format CSV.
 */

#include "core/character/CharacterState.h"

/**
 * @brief Instantané de l'état de simulation pour une ligne de télémétrie.
 *
 * Cette structure est remplie par `TelemetryRecorder::record()` à chaque pas
 * de simulation. Elle est ensuite exportée en CSV par `writeCsv()` ou utilisée
 * par les assertions de scénarios headless.
 */
struct TelemetryRow {
    double          t;                  ///< Temps de simulation (s).
    double          cm_x,   cm_vx;     ///< Position et vitesse X du CM (m ; m/s).
    double          cm_y,   cm_vy;     ///< Position et vitesse Y du CM (m ; m/s).
    double          pelvis_x;           ///< Position X du bassin (m).
    LocomotionState loco_state;         ///< Régime locomoteur courant.
    double          foot_L_x, foot_R_x; ///< Position X des pieds (m).
    double          foot_L_y, foot_R_y; ///< Position Y des pieds (m).
    bool            foot_L_on_ground, foot_R_on_ground; ///< Contact au sol.
    double          cm_target_y;        ///< Setpoint vertical du CM (m).
    double          ref_ground;         ///< Référence de sol lissée (m).
    double          ref_slope;          ///< Pente locale du terrain sous le personnage.
    double          h_ip;               ///< Hauteur nominale du pendule inversé (m).
    double          cm_offset;          ///< Décalage de hauteur additionnel (m).
    double          speed_drop;         ///< Abaissement du CM lié à la vitesse (m).
    double          slope_drop;         ///< Abaissement du CM lié à la pente (m).

    /** @deprecated Plus émis par le runtime actuel ; conservé pour les assertions historiques. */
    bool            heel_strike  = false;
};
