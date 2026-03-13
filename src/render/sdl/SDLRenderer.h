#pragma once

#include <SDL.h>
#include "../RenderState.h"

/// @brief Dessine un RenderState dans une fenêtre SDL.
///
/// Conversion monde → écran :
///   screen_x = cx + (world_x - camera_x) * scale
///   screen_y = ground_y - world_y * scale   (axe Y inversé)
class SDLRenderer {
public:
    SDLRenderer() = default;

    /// @brief Effectue le rendu d'une frame.
    /// @param renderer  pointeur SDL_Renderer valide
    /// @param state     données à dessiner
    /// @param w         largeur de la fenêtre en pixels
    /// @param h         hauteur de la fenêtre en pixels
    void render(SDL_Renderer* renderer, const RenderState& state, int w, int h) const;

private:
    void drawSegment(SDL_Renderer* r, Vec2 a, Vec2 b,
                     double camera_x, double scale, int cx, int ground_y) const;
    void drawCircle(SDL_Renderer* r, Vec2 center,
                    double radius, double camera_x, double scale,
                    int cx, int ground_y) const;
    void drawGround(SDL_Renderer* r, int w, int ground_y) const;

    SDL_Point toScreen(Vec2 world, double camera_x,
                       double scale, int cx, int ground_y) const;
};
