#pragma once

#include <SDL2/SDL.h>
#include <deque>
#include <optional>
#include "render/Camera2D.h"
#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "core/terrain/Terrain.h"
#include "config/AppConfig.h"
#include "core/character/TrailPoint.h"
#include "core/math/Vec2.h"

/**
 * @brief Renderer des overlays de debug dessinés autour du personnage.
 */
class DebugOverlayRenderer
{
public:
    /**
     * @brief Dessine les éléments de fond de debug, notamment la trail.
     * @param renderer   Renderer SDL cible.
     * @param camera     Caméra utilisée pour les conversions monde/écran.
     * @param cmConfig   Paramètres d'affichage liés au centre de masse.
     * @param trail      Historique de la trajectoire du CM.
     * @param sim_time   Temps simulé courant.
     * @param ground_y   Niveau de référence du sol.
     * @param viewport_w Largeur du viewport.
     * @param viewport_h Hauteur du viewport.
     */
    void renderBackground(SDL_Renderer*                  renderer,
                          const Camera2D&                camera,
                          const CMConfig&                cmConfig,
                          const std::deque<TrailPoint>&  trail,
                          double                         sim_time,
                          double                         ground_y,
                          int                            viewport_w,
                          int                            viewport_h) const;

    /**
     * @brief Dessine l'indicateur XCoM et la cible de pas.
     * @param xi          Position X du capture point.
     * @param target_x    Position X de la cible de pas.
     * @param trigger     Vrai si le déclencheur XCoM est actif.
     * @param show_target Active l'affichage du repère de cible.
     */
    void renderXCoM(SDL_Renderer*   renderer,
                    const Camera2D& camera,
                    double          xi,
                    double          target_x,
                    bool            trigger,
                    bool            show_target,
                    const Terrain&  terrain,
                    double          ground_y,
                    int             viewport_w,
                    int             viewport_h) const;

    /**
     * @brief Dessine les overlays de premier plan: projections, vecteurs, regard, bras.
     *
     * Cette passe lit l'état final du personnage et des options de debug, sans
     * modifier la simulation.
     */
    void renderForeground(SDL_Renderer*          renderer,
                          const Camera2D&        camera,
                          const CMState&         cm,
                          const CharacterState&  character,
                          const CharacterConfig& charConfig,
                          const HeadConfig&      headConfig,
                          const ArmConfig&       armConfig,
                          const StandingConfig&  standConfig,
                          const CMConfig&        cmConfig,
                          const Terrain&         terrain,
                          const std::optional<Vec2>& gaze_target_world,
                          double                 ref_h,
                          double                 accel_display_scale,
                          bool                   drag_active,
                          float                  drag_mouse_x,
                          float                  drag_mouse_y,
                          double                 ground_y,
                          int                    viewport_w,
                          int                    viewport_h) const;

private:
    /** @brief Utilitaire local pour dessiner un disque plein. */
    static void drawFilledCircle  (SDL_Renderer* r, float cx, float cy, float radius);
    /** @brief Utilitaire local pour dessiner un cercle filaire. */
    static void drawCircleOutline (SDL_Renderer* r, float cx, float cy, float radius);
    /** @brief Dessine la pointe d'une flèche. */
    static void drawArrowHead     (SDL_Renderer* r, float fx, float fy, float tx, float ty, float size);
    /** @brief Dessine une flèche de composante en couleur. */
    static void drawComponentArrow(SDL_Renderer* r, float fx, float fy, float tx, float ty, Uint8 r_, Uint8 g, Uint8 b);
};
