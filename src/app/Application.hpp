#pragma once

#include "app/SimulationLoop.hpp"
#include "core/debug/DebugCommandBus.hpp"
#include "core/simulation/SimulationCore.hpp"
#include "debug_ui/console/ConsoleDebugUI.hpp"
#include "debug_ui/imgui/ImGuiDebugUI.hpp"
#include "input/InputMapper.hpp"
#include "platform/sdl/SDLPlatformRuntime.hpp"
#include "render/RenderStateAdapter.hpp"
#include "render/sdl/SDLRenderer.hpp"

namespace bobtricks {

/**
 * \brief Racine d'assemblage du runtime de l'application.
 */
class Application {
public:
    Application();
    ~Application();

    /**
     * \brief Initialise le graphe runtime complet.
     */
    bool initialize();

    /**
     * \brief Execute la boucle native jusqu'a la fermeture.
     */
    int run();

    /**
     * \brief Execute une frame externe, utile pour Emscripten.
     */
    bool runFrame();

    /**
     * \brief Libere proprement toutes les ressources du runtime.
     */
    void shutdown();

private:
    bool initialized_ {false};
    bool running_ {false};
    SDLPlatformRuntime platformRuntime_ {};
    InputMapper inputMapper_ {};
    SimulationLoop simulationLoop_ {};
    SimulationCore simulationCore_ {};
    DebugCommandBus debugCommandBus_ {};
    RenderStateAdapter renderStateAdapter_ {};
    SDLRenderer renderer_ {};
    ConsoleDebugUI consoleDebugUi_;
    ImGuiDebugUI imguiDebugUi_;
    IntentRequest currentIntent_ {};
};

}  // namespace bobtricks
