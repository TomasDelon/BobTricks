#include "core/locomotion/BalanceComputer.h"
#include "core/locomotion/StandingController.h"

#include <cmath>
#include <algorithm>
#include <optional>

BalanceState computeBalanceState(const CMState&        cm,
                                 const SupportState&   support,
                                 const CharacterConfig& char_cfg,
                                 const PhysicsConfig&   phys_cfg)
{
    const std::optional<double> opt_target = computeStandingCMTarget(support, char_cfg);
    if (!opt_target) return {};  // standing geometry impossible — zero-state balance

    const double h_ref = *opt_target - support.ground_center();
    if (h_ref <= 0.0) return {};

    const double omega0 = std::sqrt(phys_cfg.gravity / h_ref);
    const double xi     = cm.position.x + cm.velocity.x / omega0;
    const double mos    = std::min(xi - support.x_left, support.x_right - xi);

    return { omega0, xi, mos };
}
