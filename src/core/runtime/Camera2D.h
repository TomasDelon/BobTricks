#pragma once

#include <SDL2/SDL.h>
#include "core/math/Vec2.h"

class Camera2D
{
public:
    struct Config {
        double base_pixels_per_meter = 120.0;
        double ground_margin_px      = 140.0;
    };

    Camera2D() = default;
    explicit Camera2D(const Config& config);

    // Move camera to world position, reset zoom
    void reset(const Vec2& center);

    // Per-frame smooth follow toward (target_x, target_y) in world coords.
    // smooth = 0 → instant. smooth > 0 → time constant in seconds.
    void update(double dt,
                double target_x, double target_y,
                bool   follow_x, bool   follow_y,
                double smooth_x, double smooth_y);

    // Pan by screen-space delta (left-click drag)
    void panByScreenDelta(float delta_x_px, float delta_y_px);

    // Zoom
    void   zoomBy(double factor);
    void   resetZoom();
    double getZoom()            const;
    double getPixelsPerMeter()  const;
    double getGroundMarginPx()  const;

    const Vec2& getCenter() const;

    // Convert world coords → screen pixel coords.
    SDL_FPoint worldToScreen(double world_x, double world_y,
                             double ground_y,
                             int viewport_w, int viewport_h) const;

    // Convert screen pixel coords → world coords.
    Vec2 screenToWorld(float screen_x, float screen_y,
                       double ground_y,
                       int viewport_w, int viewport_h) const;

private:
    Config m_config;
    Vec2   m_center       = {0.0, 0.0};
    double m_zoom         = 1.0;
};
