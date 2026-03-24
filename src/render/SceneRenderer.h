#pragma once

#include <SDL2/SDL.h>
#include "render/Camera2D.h"
#include "core/terrain/Terrain.h"

// Draws background scene elements (grid, ground/terrain).
// Everything that goes behind characters/overlay.
class SceneRenderer
{
public:
    void render(SDL_Renderer* renderer,
                const Camera2D& camera,
                const Terrain& terrain,
                double ground_y,
                int viewport_w,
                int viewport_h) const;

private:
    void drawGrid(SDL_Renderer* renderer,
                  const Camera2D& camera,
                  double ground_y,
                  int viewport_w,
                  int viewport_h) const;

    void drawGround(SDL_Renderer* renderer,
                    const Camera2D& camera,
                    const Terrain& terrain,
                    double ground_y,
                    int viewport_w,
                    int viewport_h) const;

};
