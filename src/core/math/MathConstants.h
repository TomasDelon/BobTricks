#pragma once

/**
 * @file MathConstants.h
 * @brief Constantes mathématiques partagées par tout le noyau BobTricks.
 *
 * Tous les fichiers qui définissaient localement `kPi`, `kDegToRad` ou `kTau`
 * doivent inclure ce fichier à la place pour éviter les redéfinitions.
 */

/** @brief Valeur de π (pi) avec 20 décimales significatives. */
static constexpr double kPi       = 3.14159265358979323846;
/** @brief Valeur de τ = 2π (un tour complet en radians). */
static constexpr double kTau      = kPi * 2.0;
/** @brief Facteur de conversion degrés → radians. */
static constexpr double kDegToRad = kPi / 180.0;

/** @brief Garde de stabilité numérique pour les longueurs et distances quasi-nulles. */
static constexpr double kEpsLength = 1.0e-9;
/** @brief Garde de stabilité numérique pour les angles et directions quasi-nuls. */
static constexpr double kEpsAngle  = 1.0e-6;
