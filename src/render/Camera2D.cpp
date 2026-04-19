#include "render/Camera2D.h"

#include <algorithm>
#include <cmath>

static constexpr double ZOOM_MIN = 0.1;
static constexpr double ZOOM_MAX = 5.0;

Camera2D::Camera2D(const Config& config)
    : m_config(config)
{}

void Camera2D::reset(const Vec2& center)
{
    m_center = center;
}

void Camera2D::update(double dt,
                      double target_x, double target_y,
                      bool   follow_x, bool   follow_y,
                      double smooth_x, double smooth_y,
                      double deadzone_x, double deadzone_y)
{
    auto clampToDeadzone = [](double center, double target, double deadzone) {
        const double dz = std::max(0.0, deadzone);
        if (target > center + dz) return target - dz;
        if (target < center - dz) return target + dz;
        return center;
    };

    if (follow_x) {
        const double desired_x = clampToDeadzone(m_center.x, target_x, deadzone_x);
        if (smooth_x <= 0.0)
            m_center.x = desired_x;
        else
            m_center.x += (desired_x - m_center.x) * (1.0 - std::exp(-dt / smooth_x));
    }
    if (follow_y) {
        const double desired_y = clampToDeadzone(m_center.y, target_y, deadzone_y);
        if (smooth_y <= 0.0)
            m_center.y = desired_y;
        else
            m_center.y += (desired_y - m_center.y) * (1.0 - std::exp(-dt / smooth_y));
    }
}

void Camera2D::panByScreenDelta(float delta_x_px, float delta_y_px)
{
    const double ppm = getPixelsPerMeter();
    m_center.x -= static_cast<double>(delta_x_px) / ppm;
    m_center.y += static_cast<double>(delta_y_px) / ppm;
}

void Camera2D::zoomBy(double factor)
{
    m_zoom = std::clamp(m_zoom * factor, ZOOM_MIN, ZOOM_MAX);
}

void Camera2D::resetZoom()
{
    m_zoom = 1.0;
}

SDL_FPoint Camera2D::worldToScreen(double world_x, double world_y,
                                   double ground_y,
                                   int viewport_w, int viewport_h) const
{
    const double ppm           = getPixelsPerMeter();
    const double ground_margin = m_config.ground_margin_px;
    return {
        static_cast<float>(viewport_w  * 0.5 + (world_x - m_center.x) * ppm),
        static_cast<float>(viewport_h - ground_margin - ((world_y - ground_y) - m_center.y) * ppm)
    };
}

Vec2 Camera2D::screenToWorld(float screen_x, float screen_y,
                             double ground_y,
                             int viewport_w, int viewport_h) const
{
    const double ppm           = getPixelsPerMeter();
    const double ground_margin = m_config.ground_margin_px;
    return {
        m_center.x + (static_cast<double>(screen_x) - viewport_w  * 0.5) / ppm,
        ground_y   + m_center.y + (viewport_h - ground_margin - static_cast<double>(screen_y)) / ppm
    };
}

double Camera2D::getZoom()           const { return m_zoom; }
double Camera2D::getPixelsPerMeter() const { return m_config.base_pixels_per_meter * m_zoom; }
double Camera2D::getGroundMarginPx() const { return m_config.ground_margin_px; }
const Vec2& Camera2D::getCenter()    const { return m_center; }
