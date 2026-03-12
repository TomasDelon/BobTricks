#pragma once

#include "core/debug/DebugCommandBus.hpp"

namespace bobtricks {

/**
 * \brief Facade minimale de l'interface console de debug.
 */
class ConsoleDebugUI {
public:
    explicit ConsoleDebugUI(DebugCommandBus& debugCommandBus);

    /**
     * \brief Met a jour la facade console sans blocage.
     */
    void tick();

private:
    DebugCommandBus& debugCommandBus_;
};

}  // namespace bobtricks
