#pragma once

#include <SDL2/SDL.h>
#include "render/Camera2D.h"
#include "core/terrain/Terrain.h"

/**
 * @brief Renderer du décor de fond.
 *
 * Il dessine la grille, le sol et le terrain procédural derrière le personnage
 * et les overlays de debug.
 */
class SceneRenderer
{
public:
    /** @brief Dessine tout le décor de fond. */
    void render(SDL_Renderer* renderer,
                const Camera2D& camera,
                const Terrain& terrain,
                double ground_y,
                int viewport_w,
                int viewport_h) const;

private:
    /** @brief Dessine la grille monde. */
    void drawGrid(SDL_Renderer* renderer,
                  const Camera2D& camera,
                  double ground_y,
                  int viewport_w,
                  int viewport_h) const;

    /** @brief Dessine le sol et le profil du terrain. */
    void drawGround(SDL_Renderer* renderer,
                    const Camera2D& camera,
                    const Terrain& terrain,
                    double ground_y,
                    int viewport_w,
                    int viewport_h) const;

};
