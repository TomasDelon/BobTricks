#include "app/InputController.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr double ZOOM_STEP = 1.1;
constexpr float CM_HIT_RADIUS_PX = 20.f;
constexpr float FOOT_HIT_RADIUS_PX = 15.f;
constexpr float HAND_HIT_RADIUS_PX = 15.f;

float distancePx(const SDL_FPoint& point, float mx, float my)
{
    const float dx = mx - point.x;
    const float dy = my - point.y;
    return std::sqrt(dx * dx + dy * dy);
}

} // namespace

InputController::EventResult InputController::handleEvent(const SDL_Event& event,
                                                          Camera2D& camera,
                                                          CameraConfig& cameraConfig,
                                                          SimulationCore& core,
                                                          SDL_Renderer* renderer,
                                                          double ground_y,
                                                          bool ui_captures_mouse)
{
    EventResult result;

    if (event.type == SDL_QUIT)
        result.quit_requested = true;
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        result.quit_requested = true;

    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        const bool pressed = (event.type == SDL_KEYDOWN);
        switch (event.key.keysym.sym) {
            case SDLK_q:      m_key_left  = pressed; break;
            case SDLK_d:      m_key_right = pressed; break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT: m_key_run   = pressed; break;
            case SDLK_SPACE:  if (pressed && !event.key.repeat) m_jump_requested = true; break;
            case SDLK_p:
                if (pressed && !event.key.repeat)
                    m_game_view = !m_game_view;
                break;
            default: break;
        }
    }

    if (event.type == SDL_MOUSEWHEEL && !ui_captures_mouse) {
        if (event.wheel.y > 0)      camera.zoomBy(ZOOM_STEP);
        else if (event.wheel.y < 0) camera.zoomBy(1.0 / ZOOM_STEP);
        cameraConfig.zoom = camera.getZoom();
    }

    if (event.type == SDL_MOUSEBUTTONDOWN
        && event.button.button == SDL_BUTTON_LEFT
        && !ui_captures_mouse)
    {
        int vw = 0, vh = 0;
        SDL_GetRendererOutputSize(renderer, &vw, &vh);
        m_gaze_target_world = camera.screenToWorld(
            static_cast<float>(event.button.x),
            static_cast<float>(event.button.y),
            ground_y, vw, vh);

        const SimState& s = core.state();
        bool grabbed_body_part = false;
        if (s.character.feet_initialized) {
            const float mx = static_cast<float>(event.button.x);
            const float my = static_cast<float>(event.button.y);
            const SDL_FPoint lh_s = camera.worldToScreen(
                s.character.hand_left.x, s.character.hand_left.y,
                ground_y, vw, vh);
            const SDL_FPoint rh_s = camera.worldToScreen(
                s.character.hand_right.x, s.character.hand_right.y,
                ground_y, vw, vh);
            const SDL_FPoint lf_s = camera.worldToScreen(
                s.character.foot_left.pos.x, s.character.foot_left.pos.y,
                ground_y, vw, vh);
            const SDL_FPoint rf_s = camera.worldToScreen(
                s.character.foot_right.pos.x, s.character.foot_right.pos.y,
                ground_y, vw, vh);

            float best_d = HAND_HIT_RADIUS_PX;
            enum class DragPart { None, HandLeft, HandRight, FootLeft, FootRight };
            DragPart picked = DragPart::None;

            const float dlh = distancePx(lh_s, mx, my);
            const float drh = distancePx(rh_s, mx, my);
            const float dl = distancePx(lf_s, mx, my);
            const float dr = distancePx(rf_s, mx, my);

            if (dlh <= best_d) {
                best_d = dlh;
                picked = DragPart::HandLeft;
            }
            if (drh <= best_d) {
                best_d = drh;
                picked = DragPart::HandRight;
            }
            if (dl <= std::min(best_d, FOOT_HIT_RADIUS_PX)) {
                best_d = dl;
                picked = DragPart::FootLeft;
            }
            if (dr <= std::min(best_d, FOOT_HIT_RADIUS_PX))
                picked = DragPart::FootRight;

            const Vec2 mouse_world = camera.screenToWorld(mx, my, ground_y, vw, vh);
            if (picked == DragPart::HandLeft) {
                m_dragging_hand_left = true;
                m_hand_drag_world = mouse_world;
                grabbed_body_part = true;
            } else if (picked == DragPart::HandRight) {
                m_dragging_hand_right = true;
                m_hand_drag_world = mouse_world;
                grabbed_body_part = true;
            } else if (picked == DragPart::FootLeft) {
                m_dragging_foot_left = true;
                m_foot_drag_world = mouse_world;
                grabbed_body_part = true;
            } else if (picked == DragPart::FootRight) {
                m_dragging_foot_right = true;
                m_foot_drag_world = mouse_world;
                grabbed_body_part = true;
            }
        }
        if (!grabbed_body_part)
            m_is_panning = true;
    }

    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        m_is_panning = false;
        m_dragging_foot_left = false;
        m_dragging_foot_right = false;
        m_dragging_hand_left = false;
        m_dragging_hand_right = false;
    }

    if (event.type == SDL_MOUSEMOTION && !ui_captures_mouse) {
        int vw = 0, vh = 0;
        SDL_GetRendererOutputSize(renderer, &vw, &vh);
        m_gaze_target_world = camera.screenToWorld(
            static_cast<float>(event.motion.x),
            static_cast<float>(event.motion.y),
            ground_y, vw, vh);
        if (m_dragging_foot_left || m_dragging_foot_right) {
            m_foot_drag_world = camera.screenToWorld(
                static_cast<float>(event.motion.x),
                static_cast<float>(event.motion.y),
                ground_y, vw, vh);
        } else if (m_dragging_hand_left || m_dragging_hand_right) {
            m_hand_drag_world = camera.screenToWorld(
                static_cast<float>(event.motion.x),
                static_cast<float>(event.motion.y),
                ground_y, vw, vh);
        } else if (m_is_panning) {
            const float dx = cameraConfig.follow_x ? 0.f : static_cast<float>(event.motion.xrel);
            const float dy = cameraConfig.follow_y ? 0.f : static_cast<float>(event.motion.yrel);
            if (dx != 0.f || dy != 0.f)
                camera.panByScreenDelta(dx, dy);
        }
    }

    if (event.type == SDL_MOUSEBUTTONDOWN
        && event.button.button == SDL_BUTTON_RIGHT
        && !ui_captures_mouse)
    {
        int vw = 0, vh = 0;
        SDL_GetRendererOutputSize(renderer, &vw, &vh);
        const float mx = static_cast<float>(event.button.x);
        const float my = static_cast<float>(event.button.y);
        const SimState& s = core.state();

        bool handled_body_part = false;
        if (s.character.feet_initialized) {
            const SDL_FPoint lh_s = camera.worldToScreen(
                s.character.hand_left.x, s.character.hand_left.y, ground_y, vw, vh);
            const SDL_FPoint rh_s = camera.worldToScreen(
                s.character.hand_right.x, s.character.hand_right.y, ground_y, vw, vh);
            const SDL_FPoint lf_s = camera.worldToScreen(
                s.character.foot_left.pos.x, s.character.foot_left.pos.y, ground_y, vw, vh);
            const SDL_FPoint rf_s = camera.worldToScreen(
                s.character.foot_right.pos.x, s.character.foot_right.pos.y, ground_y, vw, vh);

            float best_d = HAND_HIT_RADIUS_PX;
            enum class HitPart { None, HandLeft, HandRight, FootLeft, FootRight };
            HitPart picked = HitPart::None;

            const float dlh = distancePx(lh_s, mx, my);
            const float drh = distancePx(rh_s, mx, my);
            const float dl = distancePx(lf_s, mx, my);
            const float dr = distancePx(rf_s, mx, my);

            if (dlh <= best_d) {
                best_d = dlh;
                picked = HitPart::HandLeft;
            }
            if (drh <= best_d) {
                best_d = drh;
                picked = HitPart::HandRight;
            }
            if (dl <= std::min(best_d, FOOT_HIT_RADIUS_PX)) {
                best_d = dl;
                picked = HitPart::FootLeft;
            }
            if (dr <= std::min(best_d, FOOT_HIT_RADIUS_PX))
                picked = HitPart::FootRight;

            if (picked == HitPart::HandLeft) {
                core.toggleHandPin(true);
                handled_body_part = true;
            } else if (picked == HitPart::HandRight) {
                core.toggleHandPin(false);
                handled_body_part = true;
            } else if (picked == HitPart::FootLeft) {
                core.toggleFootPin(true);
                handled_body_part = true;
            } else if (picked == HitPart::FootRight) {
                core.toggleFootPin(false);
                handled_body_part = true;
            }
        }

        if (!handled_body_part) {
            const Vec2& cm_pos = s.cm.position;
            const SDL_FPoint cm_s = camera.worldToScreen(
                cm_pos.x, cm_pos.y, ground_y, vw, vh);
            if (distancePx(cm_s, mx, my) <= CM_HIT_RADIUS_PX) {
                m_drag_vel_active = true;
                m_drag_mouse_x = mx;
                m_drag_mouse_y = my;
            }
        }
    }

    if (event.type == SDL_MOUSEMOTION && m_drag_vel_active) {
        m_drag_mouse_x = static_cast<float>(event.motion.x);
        m_drag_mouse_y = static_cast<float>(event.motion.y);
    }

    if (event.type == SDL_MOUSEBUTTONUP
        && event.button.button == SDL_BUTTON_RIGHT
        && m_drag_vel_active)
    {
        int vw = 0, vh = 0;
        SDL_GetRendererOutputSize(renderer, &vw, &vh);
        const Vec2 mouse_world = camera.screenToWorld(
            m_drag_mouse_x, m_drag_mouse_y, ground_y, vw, vh);
        m_pending_set_velocity = mouse_world - core.state().cm.position;
        m_drag_vel_active = false;
    }

    return result;
}

InputFrame InputController::consumeInputFrame()
{
    InputFrame input;
    input.key_left = m_key_left;
    input.key_right = m_key_right;
    input.key_run = m_key_run && (m_key_left || m_key_right);
    input.jump = m_jump_requested;
    input.set_velocity = m_pending_set_velocity;
    input.gaze_target_world = m_gaze_target_world;
    m_jump_requested = false;
    m_pending_set_velocity.reset();

    if (m_dragging_foot_left) {
        input.foot_left_drag = true;
        input.foot_left_pos = m_foot_drag_world;
    }
    if (m_dragging_foot_right) {
        input.foot_right_drag = true;
        input.foot_right_pos = m_foot_drag_world;
    }
    if (m_dragging_hand_left) {
        input.hand_left_drag = true;
        input.hand_left_pos = m_hand_drag_world;
    }
    if (m_dragging_hand_right) {
        input.hand_right_drag = true;
        input.hand_right_pos = m_hand_drag_world;
    }

    return input;
}
