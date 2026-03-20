#pragma once

#include "core/math/Vec2.h"
#include "core/character/CMState.h"
#include "core/character/FootState.h"
#include "core/character/SupportState.h"
#include "core/character/BalanceState.h"
#include "core/character/StepPlan.h"
#include "core/locomotion/StepTriggerType.h"
#include "config/AppConfig.h"

enum class LocomotionState {
    Standing,    // both feet planted, geometry valid, CM recoverable
    Walking,     // intentional step sequence in progress
    Airborne,    // CM above spring contact threshold
    // NOTE: Falling was removed — the step planner always runs its own recovery.
    // There is no terminal unrecoverable state in the current architecture.
};

struct CharacterState {
    LocomotionState locomotion_state = LocomotionState::Standing;

    // Facing — persistent across frames
    double facing_vel = 0.0;   // smoothed vx (exponential filter)
    double facing     = 1.0;   // +1 = right, -1 = left

    // Derived pose — reconstructed each frame, not authoritative physics
    Vec2 pelvis       = {0.0, 0.0};
    Vec2 torso_center = {0.0, 0.0};
    Vec2 torso_top    = {0.0, 0.0};

    // Legs
    bool      feet_initialized = false;  // true after bootstrap; guards foot rendering
    Vec2      knee_left      = {0.0, 0.0};
    Vec2      knee_right     = {0.0, 0.0};
    Vec2      foot_left_eff  = {0.0, 0.0};  // clamped foot used by IK/renderer
    Vec2      foot_right_eff = {0.0, 0.0};
    FootState foot_left;
    FootState foot_right;

    // Support, balance, step
    SupportState support;
    BalanceState  balance;
    StepPlan      step_plan;

    // Walking state — per-tick event flags (reset each tick by SimulationCore)
    double          last_heel_strike_t    = -1.0;                // sim_time of last heel-strike (-1 = never)
    StepTriggerType last_trigger          = StepTriggerType::None;  // trigger intent this tick (set even if infeasible)
    bool            heel_strike_this_tick = false;               // true on the tick a swing foot lands
};

// Call at the END of stepSimulation, after integration and hard clamp.
void updateCharacterState(CharacterState& character,
                          const CMState&  cm,
                          const CharacterConfig& config,
                          const CharacterReconstructionConfig& reconstruction,
                          bool on_floor,
                          double dt);

// Derives SupportState from the two FootStates.
// Rule: support is always derived from planted feet, never from desired targets.
void updateSupportState(SupportState& support,
                        const FootState& foot_L,
                        const FootState& foot_R);
