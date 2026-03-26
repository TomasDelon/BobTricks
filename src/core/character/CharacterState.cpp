#include "core/character/CharacterState.h"
#include "core/locomotion/LegIK.h"

#include <algorithm>
#include <cmath>

static constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;

void updateCharacterState(CharacterState& ch,
                          const CMState&  cm,
                          const CharacterConfig& config,
                          const CharacterReconstructionConfig& rc,
                          bool on_floor,
                          double dt)
{
    const double vx = cm.velocity.x;

    // ── Locomotion state ─────────────────────────────────────────────────────
    // Classifies Standing/Walking/Airborne from physics state.
    // The step planner FSM in Application may promote Standing→Walking when a
    // step is active, which overrides the velocity-based Walking assignment here.
    if (!on_floor) {
        ch.locomotion_state = LocomotionState::Airborne;
    } else if (std::abs(vx) > rc.walk_eps) {
        ch.locomotion_state = LocomotionState::Walking;
    } else {
        ch.locomotion_state = LocomotionState::Standing;
    }

    // ── Facing ────────────────────────────────────────────────────────────────
    // facing_vel = smoothed vx, used only for theta (lean angle smoothness).
    // facing direction uses real vx with a deadzone to avoid flipping at rest.
    //
    // Frozen during an active step: flipping facing mid-swing would change the
    // lean direction and, at heel-strike, select the wrong foot for the next
    // step (shouldStep fires immediately after plan.active → false).
    {
        const double alpha = dt / (rc.facing_tau + dt);
        ch.facing_vel += alpha * (vx - ch.facing_vel);

        if (!ch.step_plan.active) {
            if      (vx >  rc.facing_eps) ch.facing =  1.0;
            else if (vx < -rc.facing_eps) ch.facing = -1.0;
            // else: keep previous facing
        }
    }

    // ── Lean angle from speed ─────────────────────────────────────────────────
    // signed_theta uses facing_vel (smoothed vx) with sign intact so the lean
    // passes through zero continuously when changing direction.
    // Never multiply by ch.facing here — that caused a dip+overshoot artefact.
    double signed_theta = 0.0;
    {
        const double theta_max = rc.theta_max_deg * DEG_TO_RAD;
        signed_theta = theta_max * std::tanh(ch.facing_vel / rc.v_ref);
    }

    // ── Torso spine ───────────────────────────────────────────────────────────
    // torso_dir points from pelvis toward TorsoTop (same axis as CM lean).
    // pelvis = CM - d*dir,  TorsoCenter = pelvis + L*dir,  TorsoTop = pelvis + 2L*dir
    {
        const double limb_rest = config.body_height_m / 5.0;
        const double d         = config.cm_pelvis_ratio * limb_rest;
        const double dx        = std::sin(signed_theta);
        const double dy        = std::cos(signed_theta);

        ch.pelvis.x       = cm.position.x - d               * dx;
        ch.pelvis.y       = cm.position.y - d               * dy;
        ch.torso_center.x = ch.pelvis.x   + limb_rest       * dx;
        ch.torso_center.y = ch.pelvis.y   + limb_rest       * dy;
        ch.torso_top.x    = ch.pelvis.x   + 2.0 * limb_rest * dx;
        ch.torso_top.y    = ch.pelvis.y   + 2.0 * limb_rest * dy;
    }

    // ── Knee positions (analytic IK) ──────────────────────────────────────────
    // Only computed after feet are bootstrapped; pelvis is already up-to-date.
    if (ch.feet_initialized) {
        const double L     = config.body_height_m / 5.0;
        const auto   ikL   = computeKnee(ch.pelvis, ch.foot_left.pos,  L, ch.facing);
        const auto   ikR   = computeKnee(ch.pelvis, ch.foot_right.pos, L, ch.facing);
        ch.knee_left      = ikL.knee;
        ch.knee_right     = ikR.knee;
        ch.foot_left_eff  = ikL.foot_eff;
        ch.foot_right_eff = ikR.foot_eff;
    }
}

void updateSupportState(SupportState& support,
                        const FootState& foot_L,
                        const FootState& foot_R)
{
    support.left_planted  = (foot_L.phase == FootPhase::Planted);
    support.right_planted = (foot_R.phase == FootPhase::Planted);

    if (support.left_planted && support.right_planted) {
        // Double support: interval spans both feet (sorted by x).
        if (foot_L.pos.x <= foot_R.pos.x) {
            support.x_left  = foot_L.pos.x;  support.y_left  = foot_L.ground_y;
            support.x_right = foot_R.pos.x;  support.y_right = foot_R.ground_y;
        } else {
            support.x_left  = foot_R.pos.x;  support.y_left  = foot_R.ground_y;
            support.x_right = foot_L.pos.x;  support.y_right = foot_L.ground_y;
        }
    } else if (support.left_planted) {
        // Single support: degenerate interval at left foot.
        support.x_left  = support.x_right = foot_L.pos.x;
        support.y_left  = support.y_right = foot_L.ground_y;
    } else if (support.right_planted) {
        // Single support: degenerate interval at right foot.
        support.x_left  = support.x_right = foot_R.pos.x;
        support.y_left  = support.y_right = foot_R.ground_y;
    }
    // Neither foot planted (airborne): reset to a degenerate zero-width interval
    // so that width()==0 and callers that check both_planted() get a clean state.
    // Do NOT leave stale x_left/x_right from a previous step.
    else {
        support.x_left  = support.x_right = 0.0;
        support.y_left  = support.y_right = 0.0;
    }
}
