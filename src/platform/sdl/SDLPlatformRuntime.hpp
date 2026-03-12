#pragma once

#include "platform/PlatformRuntime.hpp"

struct SDL_Window;

namespace bobtricks {

/**
 * \brief Implementation SDL de la couche plateforme.
 */
class SDLPlatformRuntime final : public PlatformRuntime {
public:
    SDLPlatformRuntime() = default;
    ~SDLPlatformRuntime() override;

    bool initialize() override;
    void shutdown() override;
    PlatformEvents pollEvents() override;
    double tickSeconds() override;
    void* getNativeWindowHandle() const override;
    WindowSize getWindowSize() const override;
    void toggleFullscreen() override;
    void setMinimumWindowSize(int width, int height) override;

    void requestMode(LocomotionMode mode);
    void requestPauseToggle();
    void requestSpeedUp();
    void requestSlowDown();
    void requestReset();
    void requestQuit();
    void requestFullscreenToggle();
    void clearPendingWebRequests();

private:
    struct PendingWebEvents {
        bool quitRequested {false};
        bool toggleFullscreenRequested {false};
        bool togglePauseRequested {false};
        bool speedUpRequested {false};
        bool slowDownRequested {false};
        bool resetRequested {false};
        std::optional<LocomotionMode> requestedMode {};
    };

    SDL_Window* windowHandle_ {nullptr};
    bool isFullscreen_ {false};
    unsigned long long lastCounter_ {0};
    PendingWebEvents pendingWebEvents_ {};
};

}  // namespace bobtricks
