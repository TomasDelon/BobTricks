#pragma once

/**
 * @file BalanceComputer.h
 * @brief Calcul du centre de masse extrapolé (XCoM) et de la marge de stabilité.
 */

#include "core/character/BalanceState.h"
#include "core/character/CMState.h"
#include "core/character/SupportState.h"
#include "config/AppConfig.h"

/**
 * @brief Calcule les indicateurs XCoM / capture point à partir de l'état courant.
 *
 * Implémente le modèle de Hof (2008) :
 * - `ω₀ = sqrt(g / h_nominal)` — fréquence propre du pendule inversé ;
 * - `ξ = x_cm + ẋ_cm / ω₀`   — centre de masse extrapolé ;
 * - `MoS = min(ξ − x_gauche, x_droite − ξ)` — marge de stabilité.
 *
 * @param cm       État cinématique courant du centre de masse.
 * @param support  Polygone de support 1D (pieds plantés).
 * @param char_cfg Configuration morphologique (nécessaire pour `h_nominal`).
 * @param phys_cfg Configuration physique (gravité `g`).
 * @return Indicateurs de stabilité remplis.
 */
BalanceState computeBalanceState(const CMState&        cm,
                                 const SupportState&   support,
                                 const CharacterConfig& char_cfg,
                                 const PhysicsConfig&   phys_cfg);
