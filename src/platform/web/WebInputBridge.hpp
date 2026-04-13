#pragma once

namespace bobtricks {

class SDLPlatformRuntime;

/**
 * \brief Pont d'entree web specifique a la build Emscripten.
 */
class WebInputBridge {
public:
    /**
     * \brief Installe le pont web pour le runtime SDL fourni.
     */
    static void install(SDLPlatformRuntime& platformRuntime);

    /**
     * \brief Desinstalle le pont web actif si necessaire.
     */
    static void uninstall(SDLPlatformRuntime& platformRuntime);
};

}  // namespace bobtricks
