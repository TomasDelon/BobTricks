#include "SDLRenderer.h"

#include <cmath>

static constexpr double PI      = 3.14159265358979323846;
static constexpr double SCALE   = 200.0;  ///< pixels par mètre
static constexpr int    CIRCLE_SEGS = 20; ///< segments pour approcher un cercle

// --------------------------------------------------------------------------
// Helpers de coordonnées
// --------------------------------------------------------------------------

SDL_Point SDLRenderer::toScreen(Vec2 world, double camera_x,
                                double scale, int cx, int ground_y) const
{
    return {
        cx      + static_cast<int>((world.x - camera_x) * scale),
        ground_y - static_cast<int>( world.y              * scale)
    };
}

void SDLRenderer::drawSegment(SDL_Renderer* r, Vec2 a, Vec2 b,
                               double camera_x, double scale,
                               int cx, int ground_y) const
{
    SDL_Point pa = toScreen(a, camera_x, scale, cx, ground_y);
    SDL_Point pb = toScreen(b, camera_x, scale, cx, ground_y);
    SDL_RenderDrawLine(r, pa.x, pa.y, pb.x, pb.y);
}

void SDLRenderer::drawCircle(SDL_Renderer* r, Vec2 center,
                              double radius, double camera_x, double scale,
                              int cx, int ground_y) const
{
    const int r_px = static_cast<int>(radius * scale);
    SDL_Point sc   = toScreen(center, camera_x, scale, cx, ground_y);

    for (int i = 0; i < CIRCLE_SEGS; ++i) {
        double a1 = 2.0 * PI * i       / CIRCLE_SEGS;
        double a2 = 2.0 * PI * (i + 1) / CIRCLE_SEGS;
        SDL_RenderDrawLine(r,
            sc.x + static_cast<int>(r_px * std::cos(a1)),
            sc.y + static_cast<int>(r_px * std::sin(a1)),
            sc.x + static_cast<int>(r_px * std::cos(a2)),
            sc.y + static_cast<int>(r_px * std::sin(a2)));
    }
}

void SDLRenderer::drawGround(SDL_Renderer* r, int w, int ground_y) const
{
    SDL_SetRenderDrawColor(r, 80, 80, 80, 255);
    SDL_RenderDrawLine(r, 0, ground_y, w, ground_y);
}

// --------------------------------------------------------------------------
// Rendu principal
// --------------------------------------------------------------------------

void SDLRenderer::render(SDL_Renderer* renderer,
                          const RenderState& state,
                          int w, int h) const
{
    const double camera_x = state.camera_pos.x;
    const double scale    = SCALE;
    const int    cx       = w / 2;
    const int    ground_y = static_cast<int>(h * 0.80);

    // Raccourci pour accéder à un nœud
    auto n = [&](NodeId id) -> Vec2 {
        return state.nodes[static_cast<int>(id)];
    };

    // --- Sol ---
    drawGround(renderer, w, ground_y);

    // --- Stickman : couleur selon le mode ---
    switch (state.mode) {
    case LocomotionMode::Walk: SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255); break;
    case LocomotionMode::Run:  SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255); break;
    default:                   SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255); break;
    }

    // Jambe gauche
    drawSegment(renderer, n(NodeId::AnkleLeft),  n(NodeId::KneeLeft),    camera_x, scale, cx, ground_y);
    drawSegment(renderer, n(NodeId::KneeLeft),   n(NodeId::TorsoBottom), camera_x, scale, cx, ground_y);

    // Jambe droite
    drawSegment(renderer, n(NodeId::AnkleRight), n(NodeId::KneeRight),   camera_x, scale, cx, ground_y);
    drawSegment(renderer, n(NodeId::KneeRight),  n(NodeId::TorsoBottom), camera_x, scale, cx, ground_y);

    // Tronc
    drawSegment(renderer, n(NodeId::TorsoBottom), n(NodeId::TorsoCenter), camera_x, scale, cx, ground_y);
    drawSegment(renderer, n(NodeId::TorsoCenter), n(NodeId::TorsoTop),    camera_x, scale, cx, ground_y);

    // Bras gauche
    drawSegment(renderer, n(NodeId::TorsoTop),  n(NodeId::ElbowLeft),  camera_x, scale, cx, ground_y);
    drawSegment(renderer, n(NodeId::ElbowLeft), n(NodeId::WristLeft),  camera_x, scale, cx, ground_y);

    // Bras droit
    drawSegment(renderer, n(NodeId::TorsoTop),   n(NodeId::ElbowRight), camera_x, scale, cx, ground_y);
    drawSegment(renderer, n(NodeId::ElbowRight), n(NodeId::WristRight), camera_x, scale, cx, ground_y);

    // Tête : cercle centré entre TorsoTop et HeadTop
    Vec2   head_center = n(NodeId::TorsoTop).lerp(n(NodeId::HeadTop), 0.5);
    double head_radius = n(NodeId::TorsoTop).distanceTo(n(NodeId::HeadTop)) * 0.5;
    drawCircle(renderer, head_center, head_radius, camera_x, scale, cx, ground_y);
}
