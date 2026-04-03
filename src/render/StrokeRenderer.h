#pragma once

#include <vector>

#include <SDL2/SDL.h>

/**
 * @brief Renderer de traits épais à partir d'une polyligne échantillonnée.
 */
class StrokeRenderer
{
public:
    /**
     * @brief Dessine une polyligne avec une épaisseur écran constante.
     * @param renderer Renderer SDL cible.
     * @param points   Polyligne déjà échantillonnée en coordonnées écran.
     * @param width_px Épaisseur voulue en pixels.
     * @param color    Couleur uniforme du trait.
     */
    void renderPolyline(SDL_Renderer* renderer,
                        const std::vector<SDL_FPoint>& points,
                        float width_px,
                        SDL_Color color) const;
};
