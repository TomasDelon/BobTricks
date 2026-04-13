#pragma once

#include "core/character/BalanceState.h"
#include "core/character/CMState.h"
#include "core/character/SupportState.h"
#include "config/AppConfig.h"

/**
 * @brief Calcule les indicateurs XCoM / capture point à partir de l'état courant.
 */
BalanceState computeBalanceState(const CMState&        cm,
                                 const SupportState&   support,
                                 const CharacterConfig& char_cfg,
                                 const PhysicsConfig&   phys_cfg);
