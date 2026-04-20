#pragma once

/**
 * @file GroundReference.h
 * @brief Référence de sol glissante utilisée pour le setpoint vertical du CM.
 *
 * Ce module calcule une hauteur de sol moyenne et une pente locale à partir
 * d'une fenêtre d'échantillonnage centrée sur le bassin du personnage. Les
 * extrémités de la fenêtre glissent de manière exponentielle pour lisser les
 * transitions sur terrain accidenté.
 */

#include "config/AppConfig.h"
#include "core/character/CharacterState.h"
#include "core/character/CMState.h"
#include "core/math/Vec2.h"
#include "core/terrain/Terrain.h"

/**
 * @brief Résultat d'un échantillonnage de la référence de sol.
 */
struct GroundReferenceSample {
    Vec2   back;       ///< Point d'échantillonnage arrière en coordonnées monde (m).
    Vec2   fwd;        ///< Point d'échantillonnage avant en coordonnées monde (m).
    double mean_y = 0.0; ///< Hauteur de sol moyenne `(back.y + fwd.y) / 2` (m).
    double slope  = 0.0; ///< Pente locale `(fwd.y − back.y) / (fwd.x − back.x)` (sans unité).
};

/**
 * @brief État persistant de la référence de sol entre deux pas de simulation.
 */
struct GroundReferenceState {
    Vec2                  pelvis_ref;            ///< Position du bassin utilisée pour cet échantillon.
    bool                  airborne_ref = false;  ///< Vrai si l'échantillon a été pris en vol.
    GroundReferenceSample sample;                ///< Dernier échantillon calculé.
};

/**
 * @brief Clamp la position X d'un point d'échantillonnage terrain dans la portée du bassin.
 *
 * Garantit que le point reste à l'intérieur du cercle de portée `radius` centré sur `pelvis`,
 * tout en conservant la direction `x_start → x_target`.
 *
 * @param terrain   Terrain procédural courant.
 * @param pelvis    Position du bassin (centre du cercle de portée).
 * @param radius    Rayon du cercle de portée (m).
 * @param x_start   Coordonnée X de départ de la fenêtre.
 * @param x_target  Coordonnée X cible souhaitée.
 * @return Coordonnée X clampée.
 */
double clampTerrainEndpointX(const Terrain& terrain,
                             const Vec2& pelvis,
                             double radius,
                             double x_start,
                             double x_target);

/**
 * @brief Calcule un nouvel échantillon de référence de sol.
 *
 * La fenêtre d'échantillonnage est définie par `TerrainSamplingConfig`.
 * Les extrémités glissent de manière exponentielle depuis leur position
 * précédente vers la cible avec la constante de temps `ts.tau_slide`.
 *
 * @param terrain       Terrain procédural courant.
 * @param pelvis        Position courante du bassin (m).
 * @param cm_x          Position X du CM (m) — pour le lookahead vitesse.
 * @param facing        Direction du personnage (`+1` droite, `-1` gauche).
 * @param speed_x       Vitesse horizontale du CM (m/s).
 * @param L             Longueur d'un segment de membre (m).
 * @param ts            Paramètres d'échantillonnage du terrain.
 * @param dt            Pas de temps de simulation (s).
 * @param left_prev_x   Position X précédente du point arrière (m).
 * @param right_prev_x  Position X précédente du point avant (m).
 * @return Nouvel échantillon de référence.
 */
GroundReferenceSample computeGroundReferenceSample(const Terrain& terrain,
                                                   const Vec2& pelvis,
                                                   double cm_x,
                                                   double facing,
                                                   double speed_x,
                                                   double L,
                                                   const TerrainSamplingConfig& ts,
                                                   double dt,
                                                   double left_prev_x,
                                                   double right_prev_x);

/**
 * @brief Met à jour la référence de sol et persiste l'état dans `CharacterState`.
 *
 * Initialise les extrémités de la fenêtre au premier appel. En vol, l'échantillon
 * est gelé à la dernière valeur au sol.
 *
 * @param ch            État du personnage (modifié en place pour persister les positions).
 * @param terrain       Terrain procédural courant.
 * @param cm            État cinématique du CM.
 * @param pelvis_ref    Position de référence du bassin pour cet appel.
 * @param airborne_ref  Vrai si le personnage est en vol.
 * @param ts            Paramètres d'échantillonnage du terrain.
 * @param L             Longueur d'un segment de membre (m).
 * @param dt            Pas de temps de simulation (s).
 * @return État de référence mis à jour.
 */
GroundReferenceState updateGroundReference(CharacterState& ch,
                                           const Terrain& terrain,
                                           const CMState& cm,
                                           const Vec2& pelvis_ref,
                                           bool airborne_ref,
                                           const TerrainSamplingConfig& ts,
                                           double L,
                                           double dt);
