#pragma once

/**
 * @file InputController.h
 * @brief Convertisseur d'événements SDL en `InputFrame` pour le noyau de simulation.
 */

#include <SDL2/SDL.h>
#include <optional>

#include "config/AppConfig.h"
#include "core/simulation/InputFrame.h"
#include "core/simulation/SimulationCore.h"
#include "render/Camera2D.h"

/**
 * @brief Traduit les événements SDL (clavier, souris) en `InputFrame`.
 *
 * Cette classe découple la couche SDL de `SimulationCore`. Elle accumule
 * l'état des touches et des gestes souris entre les frames, puis les expose
 * via `consumeInputFrame()` qui retourne (et réinitialise) l'`InputFrame`
 * courant. Elle gère également le panoramique caméra, l'ancrage des pieds et
 * des mains par glisser-déposer, et la direction du regard par clic droit.
 */
class InputController
{
public:
    /** @brief Résultat d'un traitement d'événement SDL. */
    struct EventResult {
        bool quit_requested = false; ///< Vrai si l'utilisateur a fermé la fenêtre.
    };

    /**
     * @brief Traite un événement SDL et met à jour l'état interne.
     *
     * @param event            Événement SDL à traiter.
     * @param camera           Caméra (modifiée pour le panoramique et le zoom).
     * @param cameraConfig     Configuration de la caméra (pour deadzone et suivi).
     * @param core             Noyau de simulation (pour les toggles de pieds/mains).
     * @param renderer         Renderer SDL (pour la conversion écran→monde).
     * @param ground_y         Niveau de sol monde (m).
     * @param ui_captures_mouse Vrai si ImGui capture la souris ce tick.
     * @return Résultat indiquant si l'application doit se fermer.
     */
    EventResult handleEvent(const SDL_Event& event,
                            Camera2D& camera,
                            CameraConfig& cameraConfig,
                            SimulationCore& core,
                            SDL_Renderer* renderer,
                            double ground_y,
                            bool ui_captures_mouse);

    /**
     * @brief Retourne l'`InputFrame` courant et réinitialise les drapeaux ponctuels.
     *
     * Les drapeaux persistants (touches maintenues, glissers actifs) sont conservés ;
     * seuls les drapeaux d'impulsion (saut, etc.) sont remis à zéro.
     */
    InputFrame consumeInputFrame();

    /** @brief Vrai si le mode vue jeu est actif (pas de curseur ImGui). */
    bool isGameView() const { return m_game_view; }
    /** @brief Cible de regard courante en monde, si active. */
    const std::optional<Vec2>& gazeTargetWorld() const { return m_gaze_target_world; }
    /** @brief Vrai si le glisser de vitesse (clic droit) est actif. */
    bool isVelocityDragActive() const { return m_drag_vel_active; }
    /** @brief Position X souris du glisser de vitesse (px écran). */
    float dragMouseX() const { return m_drag_mouse_x; }
    /** @brief Position Y souris du glisser de vitesse (px écran). */
    float dragMouseY() const { return m_drag_mouse_y; }

private:
    bool m_is_panning = false;

    bool m_drag_vel_active = false;
    float m_drag_mouse_x = 0.f;
    float m_drag_mouse_y = 0.f;
    std::optional<Vec2> m_pending_set_velocity;
    std::optional<Vec2> m_gaze_target_world;

    bool m_dragging_foot_left  = false;
    bool m_dragging_foot_right = false;
    Vec2 m_foot_drag_world     = {0.0, 0.0};
    bool m_dragging_hand_left  = false;
    bool m_dragging_hand_right = false;
    Vec2 m_hand_drag_world     = {0.0, 0.0};

    bool m_key_left       = false;
    bool m_key_right      = false;
    bool m_key_run        = false;
    bool m_jump_requested = false;
    bool m_game_view      = false;
};
