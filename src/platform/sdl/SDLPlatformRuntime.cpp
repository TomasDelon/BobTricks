#include "platform/sdl/SDLPlatformRuntime.hpp"

#include <SDL.h>

namespace bobtricks {

namespace {
constexpr int kWindowWidth = 960;
constexpr int kWindowHeight = 540;
}

SDLPlatformRuntime::~SDLPlatformRuntime() {
    shutdown();
}

bool SDLPlatformRuntime::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    windowHandle_ = SDL_CreateWindow(
        "BobTricks",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kWindowWidth,
        kWindowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (windowHandle_ == nullptr) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    lastCounter_ = SDL_GetPerformanceCounter();
    return true;
}

void SDLPlatformRuntime::shutdown() {
    if (windowHandle_ != nullptr) {
        SDL_DestroyWindow(windowHandle_);
        windowHandle_ = nullptr;
    }
    if (SDL_WasInit(SDL_INIT_VIDEO) != 0U) {
        SDL_Quit();
    }
    isFullscreen_ = false;
    lastCounter_ = 0;
}

PlatformEvents SDLPlatformRuntime::pollEvents() {
    PlatformEvents events;

    SDL_Event event;
    while (SDL_PollEvent(&event) == 1) {
        if (event.type == SDL_QUIT) {
            events.quitRequested = true;
            continue;
        }

        if (event.type != SDL_KEYDOWN) {
            continue;
        }

        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            events.quitRequested = true;
            break;
        case SDLK_F11:
            events.toggleFullscreenRequested = true;
            break;
        case SDLK_SPACE:
            events.togglePauseRequested = true;
            break;
        case SDLK_MINUS:
            events.slowDownRequested = true;
            break;
        case SDLK_EQUALS:
        case SDLK_PLUS:
            events.speedUpRequested = true;
            break;
        case SDLK_r:
            events.resetRequested = true;
            break;
        case SDLK_1:
            events.requestedMode = LocomotionMode::Stand;
            break;
        case SDLK_2:
            events.requestedMode = LocomotionMode::Walk;
            break;
        case SDLK_3:
            events.requestedMode = LocomotionMode::Run;
            break;
        default:
            break;
        }
    }

    return events;
}

double SDLPlatformRuntime::tickSeconds() {
    const auto currentCounter = SDL_GetPerformanceCounter();
    if (lastCounter_ == 0) {
        lastCounter_ = currentCounter;
        return 0.0;
    }

    const auto frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    const auto delta = static_cast<double>(currentCounter - lastCounter_) / frequency;
    lastCounter_ = currentCounter;
    return delta;
}

void* SDLPlatformRuntime::getNativeWindowHandle() const {
    return windowHandle_;
}

WindowSize SDLPlatformRuntime::getWindowSize() const {
    WindowSize size {};
    if (windowHandle_ != nullptr) {
        SDL_GetWindowSize(windowHandle_, &size.width, &size.height);
    }
    return size;
}

void SDLPlatformRuntime::toggleFullscreen() {
    if (windowHandle_ == nullptr) {
        return;
    }

    isFullscreen_ = !isFullscreen_;
#ifdef __EMSCRIPTEN__
    const SDL_bool mode = isFullscreen_ ? SDL_TRUE : SDL_FALSE;
#else
    const Uint32 mode = isFullscreen_ ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
#endif
    if (SDL_SetWindowFullscreen(windowHandle_, mode) != 0) {
        SDL_Log("SDL_SetWindowFullscreen failed: %s", SDL_GetError());
        isFullscreen_ = !isFullscreen_;
    }
}

void SDLPlatformRuntime::setMinimumWindowSize(int width, int height) {
#ifndef __EMSCRIPTEN__
    if (windowHandle_ != nullptr) {
        SDL_SetWindowMinimumSize(windowHandle_, width, height);
    }
#else
    (void)width;
    (void)height;
#endif
}

}  // namespace bobtricks
