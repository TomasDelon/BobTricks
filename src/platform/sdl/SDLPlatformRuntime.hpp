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

private:
    SDL_Window* windowHandle_ {nullptr};
    bool isFullscreen_ {false};
    unsigned long long lastCounter_ {0};
};

}  // namespace bobtricks
