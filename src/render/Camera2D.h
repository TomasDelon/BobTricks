#pragma once

#include <SDL2/SDL.h>
#include "core/math/Vec2.h"

/**
 * @brief Caméra 2D utilisée par le viewer SDL.
 *
 * Elle convertit les coordonnées monde <-> écran, gère le zoom et le suivi
 * du centre de masse avec éventuellement un lissage temporel.
 */
class Camera2D
{
public:
    /** @brief Paramètres statiques de conversion monde/écran. */
    struct Config {
        double base_pixels_per_meter = 120.0;
        double ground_margin_px      = 140.0;
    };

    Camera2D() = default;
    explicit Camera2D(const Config& config);

    /** @brief Repositionne la caméra et réinitialise le zoom. */
    void reset(const Vec2& center);

    /**
     * @brief Met à jour la caméra vers une cible monde.
     *
     * Les paramètres `smooth_*` sont exprimés comme constantes de temps.
     */
    void update(double dt,
                double target_x, double target_y,
                bool   follow_x, bool   follow_y,
                double smooth_x, double smooth_y,
                double deadzone_x, double deadzone_y);

    /** @brief Translate la caméra à partir d'un delta exprimé en pixels écran. */
    void panByScreenDelta(float delta_x_px, float delta_y_px);

    /** @brief Multiplie le zoom courant par un facteur. */
    void   zoomBy(double factor);
    /** @brief Réinitialise le zoom à 1.0. */
    void   resetZoom();
    double getZoom()            const;
    double getPixelsPerMeter()  const;
    double getGroundMarginPx()  const;

    const Vec2& getCenter() const;

    /** @brief Convertit une position monde en coordonnées écran. */
    SDL_FPoint worldToScreen(double world_x, double world_y,
                             double ground_y,
                             int viewport_w, int viewport_h) const;

    /** @brief Convertit une position écran en coordonnées monde. */
    Vec2 screenToWorld(float screen_x, float screen_y,
                       double ground_y,
                       int viewport_w, int viewport_h) const;

private:
    Config m_config;
    Vec2   m_center       = {0.0, 0.0};
    double m_zoom         = 1.0;
};
