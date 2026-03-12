#include "app.hpp"

#include <SDL.h>

namespace {
SDL_Window* windowHandle = nullptr;
SDL_Renderer* renderer = nullptr;
bool isRunning = false;
bool isFullscreen = false;

constexpr int windowWidth = 960;
constexpr int windowHeight = 540;

void toggleFullscreen() {
    if (windowHandle == nullptr) {
        return;
    }

    isFullscreen = !isFullscreen;
#ifdef __EMSCRIPTEN__
    const SDL_bool mode = isFullscreen ? SDL_TRUE : SDL_FALSE;
#else
    const Uint32 mode = isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
#endif
    if (SDL_SetWindowFullscreen(windowHandle, mode) != 0) {
        SDL_Log("SDL_SetWindowFullscreen failed: %s", SDL_GetError());
        isFullscreen = !isFullscreen;
    }
}
}

bool appInit() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    windowHandle = SDL_CreateWindow(
        "BobTricks",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (windowHandle == nullptr) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(windowHandle, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        renderer = SDL_CreateRenderer(windowHandle, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(windowHandle);
        windowHandle = nullptr;
        SDL_Quit();
        return false;
    }

#ifndef __EMSCRIPTEN__
    SDL_SetWindowMinimumSize(windowHandle, 320, 240);
#endif
    isRunning = true;
    return true;
}

bool appStep() {
    SDL_Event event;
    while (SDL_PollEvent(&event) == 1) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                isRunning = false;
            }
            if (event.key.keysym.sym == SDLK_F11) {
                toggleFullscreen();
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    return isRunning;
}

void appShutdown() {
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (windowHandle != nullptr) {
        SDL_DestroyWindow(windowHandle);
        windowHandle = nullptr;
    }
    SDL_Quit();
}
