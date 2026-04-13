#include "render/sdl/SDLRenderer.hpp"

#include <cmath>

#include <SDL.h>

namespace bobtricks {

namespace {
void drawFilledCircle(SDL_Renderer* renderer, Vec2 center, double radius) {
    const int xCenter = static_cast<int>(std::lround(center.x));
    const int yCenter = static_cast<int>(std::lround(center.y));
    const int radiusPixels = static_cast<int>(std::lround(radius));

    for (int yOffset = -radiusPixels; yOffset <= radiusPixels; ++yOffset) {
        const double ySquared = static_cast<double>(yOffset * yOffset);
        const double xRange = std::sqrt(static_cast<double>(radiusPixels * radiusPixels) - ySquared);
        const int xOffset = static_cast<int>(std::lround(xRange));
        SDL_RenderDrawLine(
            renderer,
            xCenter - xOffset,
            yCenter + yOffset,
            xCenter + xOffset,
            yCenter + yOffset
        );
    }
}
}

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

    SDL_SetRenderDrawColor(
        renderer_,
        renderState.ground.color.red,
        renderState.ground.color.green,
        renderState.ground.color.blue,
        renderState.ground.color.alpha
    );
    for (const auto& groundSegment : renderState.ground.tickMarks) {
        SDL_RenderDrawLine(
            renderer_,
            static_cast<int>(std::lround(groundSegment.start.x)),
            static_cast<int>(std::lround(groundSegment.start.y)),
            static_cast<int>(std::lround(groundSegment.end.x)),
            static_cast<int>(std::lround(groundSegment.end.y))
        );
    }

    SDL_SetRenderDrawColor(
        renderer_,
        renderState.skeletonColor.red,
        renderState.skeletonColor.green,
        renderState.skeletonColor.blue,
        renderState.skeletonColor.alpha
    );
    for (const auto& segment : renderState.skeletonSegments) {
        SDL_RenderDrawLine(
            renderer_,
            static_cast<int>(std::lround(segment.start.x)),
            static_cast<int>(std::lround(segment.start.y)),
            static_cast<int>(std::lround(segment.end.x)),
            static_cast<int>(std::lround(segment.end.y))
        );
    }

    if (renderState.headRadius > 0.0) {
        drawFilledCircle(renderer_, renderState.headCenter, renderState.headRadius);
    }

    SDL_SetRenderDrawColor(
        renderer_,
        renderState.jointColor.red,
        renderState.jointColor.green,
        renderState.jointColor.blue,
        renderState.jointColor.alpha
    );
    for (const auto& jointPoint : renderState.jointPoints) {
        drawFilledCircle(renderer_, jointPoint.position, jointPoint.radius);
    }

    SDL_RenderPresent(renderer_);
}

}  // namespace bobtricks
