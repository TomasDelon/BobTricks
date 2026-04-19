#pragma once

#include <SDL2/SDL.h>
#include <deque>
#include "render/Camera2D.h"
#include "core/terrain/Terrain.h"
#include "config/AppConfig.h"

struct DustParticle {
    double spawn_time = 0.0;
    double lifetime_s = 0.0;
    Vec2   pos        = {0.0, 0.0};
    Vec2   vel        = {0.0, 0.0};
    float  radius_px  = 1.0f;
    float  alpha      = 0.0f;
};

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
                const ParticlesConfig& particlesConfig,
                const std::deque<DustParticle>& dustParticles,
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
