#pragma once

#include <SDL2/SDL.h>
#include <optional>

#include "config/AppConfig.h"
#include "core/simulation/InputFrame.h"
#include "core/simulation/SimulationCore.h"
#include "render/Camera2D.h"

class InputController
{
public:
    struct EventResult {
        bool quit_requested = false;
    };

    EventResult handleEvent(const SDL_Event& event,
                            Camera2D& camera,
                            CameraConfig& cameraConfig,
                            SimulationCore& core,
                            SDL_Renderer* renderer,
                            double ground_y,
                            bool ui_captures_mouse);

    InputFrame consumeInputFrame();
    bool isGameView() const { return m_game_view; }
    const std::optional<Vec2>& gazeTargetWorld() const { return m_gaze_target_world; }
    bool isVelocityDragActive() const { return m_drag_vel_active; }
    float dragMouseX() const { return m_drag_mouse_x; }
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
