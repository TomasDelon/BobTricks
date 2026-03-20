#pragma once

#include <cmath>
#include <algorithm>

// ── Locomotion geometry utilities ─────────────────────────────────────────────
//
// All heights are measured from terrain ground level (do NOT include terrain
// offset — callers add that separately).

// Height of the CM above terrain when standing at preferred foot separation.
//
//   h_pelvis = sqrt((2L)² − (d_pref·L / 2)²)   [Pythagoras, isoceles stance]
//   result   = h_pelvis + cm_pelvis_ratio · L
//
// Uses the full leg reach r = 2L.  At d_pref = 0.90·L this gives ~2.70L,
// close to the theoretical maximum 2.75L (leg fully vertical, d = 0).
// At midstance during walking the support leg is nearly fully extended — this
// is correct and matches human biomechanics.
//
// NOT (2 + ratio)·L — that form places the pelvis at exactly 2L (full vertical
// extension only valid when d_pref = 0) and silently breaks the C4 reach
// criterion for any non-zero stance width.
//
// Parameters:
//   L               — limb segment length = body_height / 5
//   d_pref          — preferred foot separation as a fraction of L  (e.g. 0.90)
//   cm_pelvis_ratio  — CM-to-pelvis offset as a fraction of L        (e.g. 0.75)
inline double computeNominalY(double L, double d_pref, double cm_pelvis_ratio)
{
    const double half_d   = d_pref * L * 0.5;
    const double two_L    = 2.0 * L;
    const double h_pelvis = std::sqrt(std::max(0.0, two_L * two_L - half_d * half_d));
    return h_pelvis + cm_pelvis_ratio * L;
}
