#include "debug_ui/imgui/ImGuiDebugUI.hpp"

namespace bobtricks {

ImGuiDebugUI::ImGuiDebugUI(DebugCommandBus& debugCommandBus) : debugCommandBus_(debugCommandBus) {
}

void ImGuiDebugUI::tick() {
    (void)debugCommandBus_;
}

}  // namespace bobtricks
