#pragma once

#include <SDL2/SDL.h>
#include <deque>
#include "render/Camera2D.h"
#include "render/DustParticle.h"
#include "core/terrain/Terrain.h"
#include "config/AppConfig.h"

/**
 * @file SceneRenderer.h
 * @brief Renderer du décor de fond (grille, sol, terrain, poussière).
 */

/**
 * @brief Renderer du décor de fond.
 *
 * Responsable du fond dégradé, de la grille monde, du profil du terrain
 * procédural et des particules de poussière. Il est rendu avant le personnage
 * et les overlays de debug pour assurer la bonne profondeur visuelle.
 */
class SceneRenderer
{
public:
    /** @brief Dessine tout le décor de fond. */
    void render(SDL_Renderer* renderer,
                const Camera2D& camera,
                const Terrain& terrain,
                const std::deque<DustParticle>& dustParticles,
                bool show_background_grid,
                double sim_time,
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
    void drawDust(SDL_Renderer* renderer,
                  const Camera2D& camera,
                  const std::deque<DustParticle>& dustParticles,
                  double sim_time,
                  double ground_y,
                  int viewport_w,
                  int viewport_h) const;

};
