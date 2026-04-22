#pragma once

/**
 * @file BalanceState.h
 * @brief Indicateurs de stabilité issus du modèle capture point / centre de masse extrapolé.
 */

/**
 * @brief Indicateurs de stabilité dérivés du modèle capture point (Hof 2008).
 *
 * Le centre de masse extrapolé prédit si le CM peut être arrêté
 * au-dessus du polygone de support avant de tomber.
 */
struct BalanceState {
    double omega0 = 0.0; ///< Fréquence propre du pendule inversé : `sqrt(g / h_ref)` (rad/s).
    double xcom   = 0.0; ///< Centre de masse extrapolé : `ξ = x_CM + ẋ_CM / ω₀` (m).
    double mos    = 0.0; ///< Marge de stabilité : `min(ξ − x_gauche, x_droite − ξ)` (m).
};
