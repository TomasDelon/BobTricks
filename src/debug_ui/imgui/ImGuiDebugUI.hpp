#pragma once

#include "core/debug/DebugCommandBus.hpp"

namespace bobtricks {

/**
 * \brief Facade minimale de l'interface ImGui de debug.
 */
class ImGuiDebugUI {
public:
    explicit ImGuiDebugUI(DebugCommandBus& debugCommandBus);

    /**
     * \brief Met a jour l'interface ImGui minimale.
     */
    void tick();

private:
    DebugCommandBus& debugCommandBus_;
};

}  // namespace bobtricks
