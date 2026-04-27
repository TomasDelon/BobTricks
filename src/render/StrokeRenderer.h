#pragma once

#include <vector>

#include <SDL2/SDL.h>

/**
 * @file StrokeRenderer.h
 * @brief Renderer de traits épais à partir d'une polyligne SDL.
 */

/**
 * @brief Renderer de traits épais à partir d'une polyligne échantillonnée.
 *
 * SDL2 ne supporte nativement que les lignes d'épaisseur 1 px. Cette classe
 * émule des traits épais en dessinant des rectangles orientés entre chaque
 * paire de points consécutifs.
 */
class StrokeRenderer
{
public:
    /**
     * @brief Dessine une polyligne avec une épaisseur écran constante.
     *
     * @param renderer Points SDL du renderer cible.
     * @param points   Polyligne en coordonnées écran flottantes.
     * @param width_px Épaisseur du trait (px).
     * @param color    Couleur RGBA du trait.
     */
    void renderPolyline(SDL_Renderer* renderer,
                        const std::vector<SDL_FPoint>& points,
                        float width_px,
                        const SDL_Color& color) const;
};
