#include "render/sdl/SDLRenderer.hpp"

#include <SDL.h>

namespace bobtricks {

SDLRenderer::~SDLRenderer() {
    shutdown();
}

bool SDLRenderer::initialize(void* nativeWindowHandle) {
    auto* windowHandle = static_cast<SDL_Window*>(nativeWindowHandle);
    if (windowHandle == nullptr) {
        SDL_Log("SDLRenderer received a null window handle.");
        return false;
    }

    renderer_ = SDL_CreateRenderer(windowHandle, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer_ == nullptr) {
        renderer_ = SDL_CreateRenderer(windowHandle, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer_ == nullptr) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

void SDLRenderer::shutdown() {
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
}

void SDLRenderer::render(const RenderState& renderState) {
    if (renderer_ == nullptr) {
        return;
    }

    SDL_SetRenderDrawColor(
        renderer_,
        renderState.clearColor.red,
        renderState.clearColor.green,
        renderState.clearColor.blue,
        renderState.clearColor.alpha
    );
    SDL_RenderClear(renderer_);
    SDL_RenderPresent(renderer_);
}

}  // namespace bobtricks
