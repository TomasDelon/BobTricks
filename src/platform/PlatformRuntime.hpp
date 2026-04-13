#pragma once

#include "core/state/LocomotionMode.hpp"

#include <optional>

namespace bobtricks {

/**
 * \brief Taille logique courante de la fenetre.
 */
struct WindowSize {
    int width {0};
    int height {0};
};

/**
 * \brief Evenements abstraits emis par la couche plateforme.
 */
struct PlatformEvents {
    bool quitRequested {false};
    bool toggleFullscreenRequested {false};
    bool togglePauseRequested {false};
    bool speedUpRequested {false};
    bool slowDownRequested {false};
    bool resetRequested {false};
    std::optional<LocomotionMode> requestedMode {};
};

/**
 * \brief Interface de services plateforme utilises par l'application.
 */
class PlatformRuntime {
public:
    virtual ~PlatformRuntime() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual PlatformEvents pollEvents() = 0;
    virtual double tickSeconds() = 0;
    virtual void* getNativeWindowHandle() const = 0;
    virtual WindowSize getWindowSize() const = 0;
    virtual void toggleFullscreen() = 0;
    virtual void setMinimumWindowSize(int width, int height) = 0;
};

}  // namespace bobtricks
