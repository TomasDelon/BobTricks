#pragma once

// Shared mathematical constants for the BobTricks core.
// All files that previously defined kPi / kDegToRad / kTau locally should
// include this header instead.

static constexpr double kPi       = 3.14159265358979323846;
static constexpr double kTau      = kPi * 2.0;
static constexpr double kDegToRad = kPi / 180.0;

// Numerical-stability epsilons — named by use rather than magnitude.
static constexpr double kEpsLength = 1.0e-9;  // near-zero length / distance guard
static constexpr double kEpsAngle  = 1.0e-6;  // near-zero angle / direction guard
