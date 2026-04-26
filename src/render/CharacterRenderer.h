#pragma once

#include <SDL2/SDL.h>
#include "render/Camera2D.h"
#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "core/terrain/Terrain.h"
#include "config/AppConfig.h"
#include "render/StrokeRenderer.h"

/**
 * @file CharacterRenderer.h
 * @brief Renderer principal du personnage (legacy segments et splines épaisses).
 */

/**
 * @brief Renderer principal du personnage.
 *
 * Supporte deux modes de rendu sélectionnés par `SplineRenderConfig::enabled` :
 * - **Legacy** : segments SDL2 et cercles pleins pour les articulations ;
 * - **Spline** : courbes de Bézier cubiques épaisses via `StrokeRenderer`.
 *
 * Les deux modes peuvent être superposés. La méthode `render()` délègue au
 * sous-ensemble approprié de méthodes privées. Toutes les conversions monde→écran
 * passent par `Camera2D`.
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
                bool                  legacy_debug_markers,
                const Terrain&        terrain,
                double                ground_y,
                int                   viewport_w,
                int                   viewport_h,
                float                 debug_scale = 1.0f) const;

private:
    struct ScreenSpacePose {
        SDL_FPoint cm{};
        SDL_FPoint pelvis{};
        SDL_FPoint torso_center{};
        SDL_FPoint torso_top{};
        SDL_FPoint head{};
        SDL_FPoint neck{};
        SDL_FPoint elbow_left{};
        SDL_FPoint elbow_right{};
        SDL_FPoint hand_left{};
        SDL_FPoint hand_right{};
        SDL_FPoint hand_left_target{};
        SDL_FPoint hand_right_target{};
        SDL_FPoint knee_left{};
        SDL_FPoint knee_right{};
        SDL_FPoint foot_left{};
        SDL_FPoint foot_right{};
        float      reach_radius = 0.0f;
        float      head_radius  = 0.0f;
    };

    ScreenSpacePose computeScreenSpacePose(const Camera2D& camera,
                                           const CMState& cm,
                                           const CharacterState& character,
                                           const CharacterConfig& charConfig,
                                           double ground_y,
                                           int viewport_w,
                                           int viewport_h) const;

    void renderSplinePass(SDL_Renderer* renderer,
                          const Camera2D& camera,
                          const CharacterState& character,
                          const CharacterConfig& charConfig,
                          const SplineRenderConfig& splineConfig,
                          double ground_y,
                          int viewport_w,
                          int viewport_h) const;

    void renderLegacyBody(SDL_Renderer* renderer,
                          const CharacterState& character,
                          const ScreenSpacePose& pose,
                          float debug_scale) const;
    void renderVisualLayer(SDL_Renderer* renderer,
                           const Camera2D& camera,
                           const CharacterState& character,
                           const CharacterConfig& charConfig,
                           const SplineRenderConfig& splineConfig,
                           double ground_y,
                           int viewport_w,
                           int viewport_h) const;
    void renderLegacyDebugLayer(SDL_Renderer* renderer,
                                const Camera2D& camera,
                                const CharacterState& character,
                                const CharacterConfig& charConfig,
                                const CMConfig& cmConfig,
                                const Terrain& terrain,
                                const ScreenSpacePose& pose,
                                double ground_y,
                                int viewport_w,
                                int viewport_h,
                                float debug_scale) const;

    void renderDebugMarkersBeforeBody(SDL_Renderer* renderer,
                                      const Camera2D& camera,
                                      const CharacterState& character,
                                      const CharacterConfig& charConfig,
                                      const CMConfig& cmConfig,
                                      const Terrain& terrain,
                                      const ScreenSpacePose& pose,
                                      double ground_y,
                                      int viewport_w,
                                      int viewport_h,
                                      float debug_scale) const;

    void renderDebugMarkersAfterBody(SDL_Renderer* renderer,
                                     const Camera2D& camera,
                                     const CharacterState& character,
                                     const ScreenSpacePose& pose,
                                     double ground_y,
                                     int viewport_w,
                                     int viewport_h,
                                     float debug_scale) const;

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
