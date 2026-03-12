#pragma once

#include "input/IntentRequest.hpp"
#include "platform/PlatformRuntime.hpp"

namespace bobtricks {

/**
 * \brief Traduit les evenements abstraits de plateforme en intention de locomotion.
 */
class InputMapper {
public:
    /**
     * \brief Met a jour l'intention courante a partir des evenements plateforme.
     */
    IntentRequest map(const PlatformEvents& events, const IntentRequest& previousIntent) const;
};

}  // namespace bobtricks
