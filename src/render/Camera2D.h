#pragma once

#include <SDL2/SDL.h>
#include "core/math/Vec2.h"

/**
 * @file Camera2D.h
 * @brief Caméra 2D avec suivi, deadzone et conversion monde/écran.
 */

/**
 * @brief Caméra 2D utilisée par le viewer SDL.
 *
 * Elle convertit les coordonnées monde ↔ écran, gère le zoom et le suivi
 * du centre de masse avec une deadzone configurable et un lissage temporel
 * exponentiel. L'axe Y est inversé entre le repère monde (Y vers le haut) et
 * le repère écran (Y vers le bas).
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

    /**
     * @brief Repositionne la caméra sur `center` et réinitialise le zoom à 1.
     * @param center Position monde du centre de la vue (m).
     */
    void reset(const Vec2& center);

    /**
     * @brief Met à jour la position de la caméra vers une cible monde.
     *
     * Le suivi est activé séparément pour X et Y. Quand `smooth_*` est nul,
     * le mouvement est instantané. La deadzone empêche la caméra de bouger tant
     * que la cible reste dans une zone centrale de demi-largeur `deadzone_*`.
     *
     * @param dt          Pas de temps (s).
     * @param target_x    Cible X en monde (m).
     * @param target_y    Cible Y en monde (m).
     * @param follow_x    Active le suivi horizontal.
     * @param follow_y    Active le suivi vertical.
     * @param smooth_x    Constante de temps du lissage X (s ; 0 = instantané).
     * @param smooth_y    Constante de temps du lissage Y (s).
     * @param deadzone_x  Demi-largeur de la deadzone horizontale (m).
     * @param deadzone_y  Demi-hauteur de la deadzone verticale (m).
     */
    void update(double dt,
                double target_x, double target_y,
                bool   follow_x, bool   follow_y,
                double smooth_x, double smooth_y,
                double deadzone_x, double deadzone_y);

    /**
     * @brief Translate la caméra à partir d'un delta exprimé en pixels écran.
     * @param delta_x_px Déplacement horizontal en pixels.
     * @param delta_y_px Déplacement vertical en pixels.
     */
    void panByScreenDelta(float delta_x_px, float delta_y_px);

    /** @brief Multiplie le zoom courant par `factor`. */
    void   zoomBy(double factor);
    /** @brief Réinitialise le zoom à 1.0. */
    void   resetZoom();
    /** @brief Retourne le zoom courant. */
    double getZoom()            const;
    /** @brief Retourne le facteur pixels/mètre effectif (inclut le zoom). */
    double getPixelsPerMeter()  const;
    /** @brief Retourne la marge de sol en pixels. */
    double getGroundMarginPx()  const;
    /** @brief Retourne la position monde du centre de la vue (m). */
    const Vec2& getCenter() const;

    /**
     * @brief Convertit une position monde en coordonnées pixel écran.
     * @param world_x    Position X monde (m).
     * @param world_y    Position Y monde (m).
     * @param ground_y   Niveau de sol monde (m) — référence verticale.
     * @param viewport_w Largeur du viewport SDL (px).
     * @param viewport_h Hauteur du viewport SDL (px).
     * @return Point flottant en coordonnées écran.
     */
    SDL_FPoint worldToScreen(double world_x, double world_y,
                             double ground_y,
                             int viewport_w, int viewport_h) const;

    /**
     * @brief Convertit une position écran en coordonnées monde.
     * @param screen_x   X écran (px).
     * @param screen_y   Y écran (px).
     * @param ground_y   Niveau de sol monde (m).
     * @param viewport_w Largeur du viewport (px).
     * @param viewport_h Hauteur du viewport (px).
     * @return Position monde (m).
     */
    Vec2 screenToWorld(float screen_x, float screen_y,
                       double ground_y,
                       int viewport_w, int viewport_h) const;

private:
    Config m_config;
    Vec2   m_center       = {0.0, 0.0};
    double m_zoom         = 1.0;
};
