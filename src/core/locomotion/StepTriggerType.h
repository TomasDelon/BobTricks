#pragma once

/**
 * @file StepTriggerType.h
 * @brief Enumération des causes possibles de déclenchement d'un pas.
 */

/**
 * @brief Type de déclencheur ayant provoqué une demande de pas ce tick.
 *
 * Affiché dans le panneau **Balance** du debug UI pour l'inspection en temps réel.
 */
enum class StepTriggerType {
    None,      ///< Aucun pas déclenché ce tick.
    Normal,    ///< Pied arrière trop loin derrière le bassin à vitesse de marche.
    Emergency  ///< XCoM (capture point) a dépassé le pied avant — risque de chute.
};
