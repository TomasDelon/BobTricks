#pragma once

/**
 * @file Geometry.h
 * @brief Utilitaires géométriques partagés par la locomotion.
 *
 * Ce fichier fournit des fonctions en ligne de faible coût, basées sur la
 * géométrie du squelette, qui sont nécessaires à plusieurs modules de
 * locomotion sans créer de dépendances cycliques.
 */

// Hauteur du CM au-dessus du terrain en posture debout avec l'ecartement prefere.
//
//   h_pelvis = sqrt((2L)² − (d_pref·L / 2)²)   [Pythagore, posture isocele]
//   resultat = h_pelvis + cm_pelvis_ratio · L
//
// Utilise la portee complete de la jambe r = 2L. Pour d_pref = 0.90·L, on obtient
// environ 2.70L, proche du maximum theorique 2.75L (jambe verticale, d = 0).
// En milieu d'appui pendant la marche, la jambe de support est presque etendue :
// c'est voulu et coherent avec la biomecanique humaine.
//
// PAS (2 + ratio)·L : cette forme place le bassin exactement a 2L (extension
// verticale complete seulement valide quand d_pref = 0) et casse silencieusement
// le critere de portee C4 pour tout ecartement non nul.
//
// Parametres :
//   L                — longueur d'un segment de membre = body_height / 5
//   d_pref           — ecartement prefere des pieds en fraction de L (ex. 0.90)
//   cm_pelvis_ratio  — decalage CM-bassin en fraction de L           (ex. 0.75)
/**
 * @brief Calcule la hauteur nominale du centre de masse au-dessus du terrain.
 *
 * Formule : `h_pelvis = sqrt((2L)² − (d_pref·L / 2)²)`, puis `h = h_pelvis + ratio·L`.
 * Le calcul suppose deux jambes de longueur totale `2L` en posture préférée.
 *
 * @param L              Longueur d'un segment de membre (`= taille / 5`).
 * @param d_pref         Séparation préférée des pieds en fractions de `L` (ex. 0.90).
 * @param cm_pelvis_ratio Décalage CM→bassin en fractions de `L` (ex. 0.75).
 * @return Hauteur nominale du CM au-dessus du sol (en mètres).
 *
 * @pre `L > 0`, `d_pref >= 0`, `cm_pelvis_ratio >= 0`.
 */
double computeNominalY(double L, double d_pref, double cm_pelvis_ratio);
