#include "app.hpp"

#include <SDL.h>

#include "core/simulation/SimulationCore.h"
#include "core/state/IntentRequest.h"
#include "render/RenderState.h"
#include "render/sdl/SDLRenderer.h"

namespace {

SDL_Window*   windowHandle = nullptr;
SDL_Renderer* sdlRenderer  = nullptr;
bool          isRunning    = false;
bool          isFullscreen = false;

constexpr int    WIN_W = 960;
constexpr int    WIN_H = 540;
constexpr double DT    = 1.0 / 60.0;

SimulationCore simCore;
SDLRenderer    gRenderer;
IntentRequest  intent;

void toggleFullscreen()
{
    if (!windowHandle) return;
    isFullscreen = !isFullscreen;
    const Uint32 mode = isFullscreen ? static_cast<Uint32>(SDL_WINDOW_FULLSCREEN_DESKTOP) : 0u;
    if (SDL_SetWindowFullscreen(windowHandle, mode) != 0) {
        SDL_Log("SDL_SetWindowFullscreen failed: %s", SDL_GetError());
        isFullscreen = !isFullscreen;
    }
}

} // namespace

// --------------------------------------------------------------------------

bool appInit()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    windowHandle = SDL_CreateWindow(
        "BobTricks",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!windowHandle) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    sdlRenderer = SDL_CreateRenderer(windowHandle, -1,
                      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer)
        sdlRenderer = SDL_CreateRenderer(windowHandle, -1, SDL_RENDERER_SOFTWARE);
    if (!sdlRenderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(windowHandle);
        SDL_Quit();
        return false;
    }

    SDL_SetWindowMinimumSize(windowHandle, 320, 240);
    simCore   = SimulationCore{};
    intent    = IntentRequest{};
    isRunning = true;
    return true;
}

// --------------------------------------------------------------------------

bool appStep()
{
    // --- Événements ---
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: isRunning = false;                             break;
            case SDLK_F11:    toggleFullscreen();                            break;
            case SDLK_s:      intent.requested_mode = LocomotionMode::Stand; break;
            case SDLK_w:      intent.requested_mode = LocomotionMode::Walk;  break;
            case SDLK_r:      intent.requested_mode = LocomotionMode::Run;   break;
            default: break;
            }
        }
    }

    // --- Simulation (pas fixe 60 Hz) ---
    simCore.step(DT, intent);

    // --- Construction du RenderState ---
    const CharacterState& cs = simCore.getState();
    RenderState rs;
    rs.nodes      = cs.node_positions;
    rs.camera_pos = cs.cm.procedural.target_position;
    rs.mode       = cs.mode;
    rs.gait_phase = cs.gait_phase;

    // --- Rendu ---
    int w = 0, h = 0;
    SDL_GetRendererOutputSize(sdlRenderer, &w, &h);

    SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
    SDL_RenderClear(sdlRenderer);

    gRenderer.render(sdlRenderer, rs, w, h);

    SDL_RenderPresent(sdlRenderer);
    return isRunning;
}

// --------------------------------------------------------------------------

void appShutdown()
{
    if (sdlRenderer)  { SDL_DestroyRenderer(sdlRenderer); sdlRenderer  = nullptr; }
    if (windowHandle) { SDL_DestroyWindow(windowHandle);  windowHandle = nullptr; }
    SDL_Quit();
}
