#pragma once

#include <SDL2/SDL.h>
#include "render/Camera2D.h"
#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "core/terrain/Terrain.h"
#include "config/AppConfig.h"
#include "render/StrokeRenderer.h"

/**
 * @brief Renderer principal du personnage.
 *
 * Il sait dessiner à la fois la représentation legacy en segments et la
 * représentation spline épaisse, en s'appuyant uniquement sur `CharacterState`.
 */
class CharacterRenderer
{
public:
    /** @brief Dessine le personnage complet dans le viewport courant. */
    void render(SDL_Renderer*         renderer,
                const Camera2D&       camera,
                const CMState&        cm,
                const CharacterState& character,
                const CharacterConfig& charConfig,
                const SplineRenderConfig& splineConfig,
                const CharacterReconstructionConfig& reconstruction,
                const CMConfig&       cmConfig,
                bool                  spline_only,
                const Terrain&        terrain,
                double                ground_y,
                int                   viewport_w,
                int                   viewport_h) const;

private:
    /** @brief Dessine un disque plein en coordonnées écran. */
    static void drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius);
    /** @brief Dessine un cercle filaire en coordonnées écran. */
    static void drawCircleOutline(SDL_Renderer* renderer, float cx, float cy, float radius);

    /** @brief Dessine une courbe de test pour le renderer spline. */
    void renderSplineTest(SDL_Renderer* renderer,
                          const Camera2D& camera,
                          const CharacterState& character,
                          const CharacterConfig& charConfig,
                          const SplineRenderConfig& splineConfig,
                          double ground_y,
                          int viewport_w,
                          int viewport_h) const;
    /** @brief Dessine la tête en contour spline fermé. */
    void renderSplineHead(SDL_Renderer* renderer,
                          const Camera2D& camera,
                          const CharacterState& character,
                          const SplineRenderConfig& splineConfig,
                          double ground_y,
                          int viewport_w,
                          int viewport_h) const;
    /** @brief Dessine le tronc sous forme de spline continue. */
    void renderSplineTorso(SDL_Renderer* renderer,
                           const Camera2D& camera,
                           const CharacterState& character,
                           const SplineRenderConfig& splineConfig,
                           double ground_y,
                           int viewport_w,
                           int viewport_h) const;
    /** @brief Dessine un bras à partir des articulations reconstruites. */
    void renderSplineArm(SDL_Renderer* renderer,
                         const Camera2D& camera,
                         const Vec2& shoulder,
                         const Vec2& elbow,
                         const Vec2& hand,
                         int depth_index,
                         const SplineRenderConfig& splineConfig,
                         double ground_y,
                         int viewport_w,
                         int viewport_h) const;
    /** @brief Dessine une jambe à partir des articulations reconstruites. */
    void renderSplineLeg(SDL_Renderer* renderer,
                         const Camera2D& camera,
                         const Vec2& pelvis,
                         const Vec2& knee,
                         const Vec2& foot,
                         int depth_index,
                         const SplineRenderConfig& splineConfig,
                         double ground_y,
                         int viewport_w,
                         int viewport_h) const;

    StrokeRenderer m_strokeRenderer;
};
