#pragma once

// World-space constants shared across the simulation core and the SDL viewer.
// Keeping them here avoids duplicating the value between Application.cpp and
// any future headless runner that has no SDL dependency.

namespace World {

// Baseline Y of the terrain geometry in world coordinates.
// Terrain heights are relative to this value.
inline constexpr double GROUND_Y = 0.0;

} // namespace World
