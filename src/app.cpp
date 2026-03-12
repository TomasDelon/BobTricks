#include "app.hpp"

#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace {
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
bool g_running = false;

constexpr int kWindowWidth = 960;
constexpr int kWindowHeight = 540;
constexpr int kPlatformWidth = 220;
constexpr int kPlatformHeight = 24;
constexpr int kPlatformY = 380;
constexpr int kSquareSize = 56;

SDL_Rect g_leftPlatform {110, kPlatformY, kPlatformWidth, kPlatformHeight};
SDL_Rect g_rightPlatform {620, kPlatformY, kPlatformWidth, kPlatformHeight};
SDL_Rect g_square {};
SDL_Point g_dragOffset {0, 0};
bool g_dragging = false;
bool g_onTargetPlatform = false;

void updateWindowTitle() {
    if (g_window == nullptr) {
        return;
    }
    const char* title = g_onTargetPlatform ? "BobTricks - Success" : "BobTricks";
    SDL_SetWindowTitle(g_window, title);
#ifdef __EMSCRIPTEN__
    MAIN_THREAD_EM_ASM({
        document.title = UTF8ToString($0);
        window.__bobtricksSuccess = UTF8ToString($0) === "BobTricks - Success";
    }, title);
#endif
}

bool pointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h;
}

bool squareCanLandOnPlatform(const SDL_Rect& square, const SDL_Rect& platform) {
    const int squareCenterX = square.x + square.w / 2;
    const int squareBottomY = square.y + square.h;
    const bool horizontallyAligned =
        squareCenterX >= platform.x && squareCenterX <= platform.x + platform.w;
    const bool verticallyClose =
        squareBottomY >= platform.y - 24 && squareBottomY <= platform.y + platform.h;
    return horizontallyAligned && verticallyClose;
}

void placeSquareOnPlatform(const SDL_Rect& platform) {
    g_square.w = kSquareSize;
    g_square.h = kSquareSize;
    g_square.x = platform.x + (platform.w - g_square.w) / 2;
    g_square.y = platform.y - g_square.h;
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
        SDL_WINDOW_SHOWN
    );
    if (g_window == nullptr) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    g_renderer = SDL_CreateRenderer(
        g_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
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

    placeSquareOnPlatform(g_leftPlatform);
    g_dragging = false;
    g_onTargetPlatform = false;
    updateWindowTitle();
    g_running = true;
    return true;
}

bool appStep() {
    SDL_Event event;
    while (SDL_PollEvent(&event) == 1) {
        if (event.type == SDL_QUIT) {
            g_running = false;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            g_running = false;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            if (pointInRect(event.button.x, event.button.y, g_square)) {
                g_dragging = true;
                g_dragOffset.x = event.button.x - g_square.x;
                g_dragOffset.y = event.button.y - g_square.y;
            }
        }
        if (event.type == SDL_MOUSEMOTION && g_dragging) {
            g_square.x = event.motion.x - g_dragOffset.x;
            g_square.y = event.motion.y - g_dragOffset.y;
        }
        if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && g_dragging) {
            g_dragging = false;
            g_square.x = event.button.x - g_dragOffset.x;
            g_square.y = event.button.y - g_dragOffset.y;

            if (squareCanLandOnPlatform(g_square, g_rightPlatform)) {
                placeSquareOnPlatform(g_rightPlatform);
                g_onTargetPlatform = true;
                updateWindowTitle();
            } else if (squareCanLandOnPlatform(g_square, g_leftPlatform)) {
                placeSquareOnPlatform(g_leftPlatform);
                g_onTargetPlatform = false;
                updateWindowTitle();
            } else if (g_onTargetPlatform) {
                placeSquareOnPlatform(g_rightPlatform);
                updateWindowTitle();
            } else {
                placeSquareOnPlatform(g_leftPlatform);
                updateWindowTitle();
            }
        }
    }

    SDL_SetRenderDrawColor(g_renderer, 18, 24, 38, 255);
    SDL_RenderClear(g_renderer);

    SDL_Rect lane {
        0,
        kPlatformY + kPlatformHeight,
        kWindowWidth,
        kWindowHeight - (kPlatformY + kPlatformHeight)
    };
    SDL_SetRenderDrawColor(g_renderer, 12, 16, 26, 255);
    SDL_RenderFillRect(g_renderer, &lane);

    SDL_SetRenderDrawColor(g_renderer, 55, 110, 220, 255);
    SDL_RenderFillRect(g_renderer, &g_leftPlatform);
    SDL_RenderFillRect(g_renderer, &g_rightPlatform);

    if (g_onTargetPlatform) {
        SDL_SetRenderDrawColor(g_renderer, 110, 190, 255, 255);
        SDL_Rect glow {
            g_rightPlatform.x - 6,
            g_rightPlatform.y - 6,
            g_rightPlatform.w + 12,
            g_rightPlatform.h + 12
        };
        SDL_RenderDrawRect(g_renderer, &glow);
    }

    SDL_SetRenderDrawColor(g_renderer, 230, 90, 70, 255);
    SDL_RenderFillRect(g_renderer, &g_square);

    if (g_dragging) {
        SDL_SetRenderDrawColor(g_renderer, 255, 220, 210, 255);
        SDL_RenderDrawRect(g_renderer, &g_square);
    }

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
