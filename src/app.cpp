#include "app.hpp"

#include <SDL.h>

namespace {
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
bool g_running = false;
bool g_fullscreen = false;

constexpr int kWindowWidth = 960;
constexpr int kWindowHeight = 540;

void toggleFullscreen() {
    if (g_window == nullptr) {
        return;
    }

    g_fullscreen = !g_fullscreen;
#ifdef __EMSCRIPTEN__
    const SDL_bool mode = g_fullscreen ? SDL_TRUE : SDL_FALSE;
#else
    const Uint32 mode = g_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
#endif
    if (SDL_SetWindowFullscreen(g_window, mode) != 0) {
        SDL_Log("SDL_SetWindowFullscreen failed: %s", SDL_GetError());
        g_fullscreen = !g_fullscreen;
    }
}
}

bool appInit() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    g_window = SDL_CreateWindow(
        "BobTricks",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kWindowWidth,
        kWindowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (g_window == nullptr) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (g_renderer == nullptr) {
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (g_renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
        SDL_Quit();
        return false;
    }

#ifndef __EMSCRIPTEN__
    SDL_SetWindowMinimumSize(g_window, 320, 240);
#endif
    g_running = true;
    return true;
}

bool appStep() {
    SDL_Event event;
    while (SDL_PollEvent(&event) == 1) {
        if (event.type == SDL_QUIT) {
            g_running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                g_running = false;
            }
            if (event.key.keysym.sym == SDLK_F11) {
                toggleFullscreen();
            }
        }
    }

    SDL_SetRenderDrawColor(g_renderer, 20, 20, 20, 255);
    SDL_RenderClear(g_renderer);
    SDL_RenderPresent(g_renderer);

    return g_running;
}

void appShutdown() {
    if (g_renderer != nullptr) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = nullptr;
    }
    if (g_window != nullptr) {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }
    SDL_Quit();
}
