#pragma once

/**
 * @file LegIK.h
 * @brief IK analytique à deux segments pour les jambes (loi des cosinus).
 */

#include "core/math/Vec2.h"

/**
 * @brief Résultat d'une IK analytique de jambe à deux segments.
 */
struct LegIKResult {
    Vec2 knee;     ///< Position du genou calculée (m).
    Vec2 foot_eff; ///< Position effective du pied : égale à `F` si atteignable, sinon clampée sur le cercle de portée.
};

/**
 * @brief Calcule la position du genou et du pied effectif pour une jambe.
 *
 * Résout le problème IK par la loi des cosinus. Si la cible dépasse la portée
 * maximale `2L`, le pied est clampé et le genou est aligné avec la direction
 * bassin→pied.
 *
 * @param P      Position du bassin (origine de la jambe) en coordonnées monde (m).
 * @param F      Position cible du pied en coordonnées monde (m).
 * @param L      Longueur d'un segment de membre (`= taille / 5`) (m).
 * @param facing Direction du personnage (`+1` = droite, `-1` = gauche).
 * @return Résultat contenant la position du genou et le pied effectif.
 */
LegIKResult computeKnee(Vec2 P, Vec2 F, double L, double facing);
