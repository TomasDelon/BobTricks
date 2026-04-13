#include "debug_ui/console/ConsoleDebugUI.hpp"

namespace bobtricks {

ConsoleDebugUI::ConsoleDebugUI(DebugCommandBus& debugCommandBus) : debugCommandBus_(debugCommandBus) {
}

void ConsoleDebugUI::tick() {
    (void)debugCommandBus_;
}

}  // namespace bobtricks
