#include "core/simulation/SimulationCore.h"

#include "core/simulation/SimulationCoreInternal.h"
#include "core/simulation/SimVerbosity.h"

#include <algorithm>
#include <cmath>

using namespace simcore_detail;

void SimulationCore::stepRetargetLateSwings(StepCtx& ctx)
{
    if (!ctx.landing_recovery_active) return;

    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    retargetSwingIfLate(ch.foot_left,  ch.foot_right, ch, m_terrain, ctx.pelvis,
                        ctx.eff_walk, ctx.eff_step, cm.position.x, cm.velocity.x,
                        ctx.reach_radius, ctx.L, ctx.ref_slope,
                        ctx.landing_recovery_gain, ctx.speed_abs, ctx.max_spd);
    retargetSwingIfLate(ch.foot_right, ch.foot_left, ch, m_terrain, ctx.pelvis,
                        ctx.eff_walk, ctx.eff_step, cm.position.x, cm.velocity.x,
                        ctx.reach_radius, ctx.L, ctx.ref_slope,
                        ctx.landing_recovery_gain, ctx.speed_abs, ctx.max_spd);
}

void SimulationCore::stepHandleJumpFlight(StepCtx& ctx)
{
    if (!m_state.character.jump_flight_active) return;

    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    updateJumpLandingTargets(ch, m_config, m_terrain, m_config.reconstruction,
                             cm, ctx.g, ctx.L);
    const double progress = 1.0 - ch.jump_time_remaining
                                  / std::max(ch.jump_total_flight_time, 1.0e-4);
    updateJumpFeetInFlight(ch, ctx.pelvis, progress);

    if (ctx.airborne_final) {
        releaseFeetAirborne(ch);
        ch.foot_left.airborne  = true;
        ch.foot_right.airborne = true;
    }
}

void SimulationCore::stepHandleGroundRecontact(StepCtx& ctx)
{
    CharacterState& ch = m_state.character;

    if (ctx.airborne_final) {
        if (!ch.jump_flight_active)
            releaseFeetAirborne(ch);
        return;
    }
    if (!ctx.airborne_ref || ctx.airborne_final == ctx.airborne_ref) return;

    const double impact_speed = std::max(0.0, -ctx.pre_vertical_velocity);
    const double impact_ratio = std::clamp(
        impact_speed / std::max(m_config.physics.jump_impulse, 1.0e-4), 0.0, 2.0);

    if (ch.jump_flight_active && ch.jump_targets_valid) {
        plantFootAtTarget(ch.foot_left,  ch.jump_left_target);
        plantFootAtTarget(ch.foot_right, ch.jump_right_target);
        ch.jump_flight_active    = false;
        ch.jump_preload_active   = false;
        ch.jump_targets_valid    = false;
        ch.landing_recovery_timer = m_config.jump.landing_dur_jump;
        ch.landing_recovery_boost = m_config.jump.landing_boost_base_jump
                                  + m_config.jump.landing_boost_scale_jump * impact_ratio;
        return;
    }

    bootstrapFeetOnLanding(ch, m_config.standing, m_terrain,
                           ctx.pelvis, ctx.reach_radius, ctx.L);
    ch.landing_recovery_timer = m_config.jump.landing_dur_walk;
    ch.landing_recovery_boost = m_config.jump.landing_boost_base_walk
                              + m_config.jump.landing_boost_scale_walk * impact_ratio;
}

void SimulationCore::stepFireRunTrigger(StepCtx& ctx)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    const bool back_is_left = (ch.facing >= 0.0) ? (ch.foot_left.pos.x < ch.foot_right.pos.x)
                                                 : (ch.foot_left.pos.x > ch.foot_right.pos.x);
    FootState& back_foot  = back_is_left ? ch.foot_left : ch.foot_right;
    FootState& front_foot = (&back_foot == &ch.foot_left) ? ch.foot_right : ch.foot_left;

    const double back_extent      = std::lerp(0.40 * ctx.L, 0.80 * ctx.L, ctx.run_timing.speed_ratio);
    const double movement_back_x  = ctx.pelvis.x - ch.facing * back_extent;
    const bool   outside_back_win = (back_foot.pos.x - movement_back_x) * ch.facing < 0.0;
    const double behind_back      = (ctx.pelvis.x - back_foot.pos.x) * ch.facing;
    const double trigger_dist     = std::lerp(0.45 * ctx.L, 0.75 * ctx.L, ctx.run_timing.speed_ratio);
    if (!outside_back_win && behind_back <= trigger_dist) return;

    const double tx = computeRunLandingX(m_config.run, ch, front_foot,
                                         ctx.pelvis, cm.velocity.x,
                                         ctx.reach_radius, ctx.L, ctx.run_timing,
                                         ctx.ref_slope);
    beginSwingStep(back_foot, front_foot, ch, tx, m_terrain,
                   false, ctx.L, ctx.eff_step, ctx.eff_walk, ctx.speed_abs, ctx.max_spd);
    const double uphill_alignment = ch.facing * ctx.ref_slope;
    const double step_rise = back_foot.swing_target.y - back_foot.swing_start.y;
    if (uphill_alignment > 0.0 && step_rise > 0.0) {
        const double uphill_strength = std::clamp(uphill_alignment / 0.35, 0.0, 1.0);
        back_foot.swing_h_clear += std::lerp(0.0, 0.16 * ctx.L, uphill_strength);
    }
    ch.step_cooldown = 0.0;
}

void SimulationCore::stepFireWalkTrigger(StepCtx& ctx)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    const StepTriggerEval trigger_eval =
        evaluateStepTriggers(ch, ctx.eff_lx, ctx.eff_rx, ctx.xi, cm.velocity.x,
                             ctx.eff_walk.eps_step, ctx.pelvis, ctx.L,
                             ctx.eff_walk.d_rear_max);

    if (!trigger_eval.xcom_trigger && !trigger_eval.rear_trigger) return;

    const bool no_forward_input = std::abs(ctx.input_dir) <= 0.01;
    const bool corrective       = no_forward_input
                               && trigger_eval.rear_trigger
                               && !trigger_eval.xcom_trigger;
    stepLaunchSwing(trigger_eval.step_left_xcom, corrective, ctx);
    simLog("Step trigger: %s  d_rear=%.2fL  xcom=%d  rear=%d\n",
           trigger_eval.step_left_xcom ? "LEFT" : "RIGHT",
           trigger_eval.d_rear / ctx.L,
           static_cast<int>(trigger_eval.xcom_trigger),
           static_cast<int>(trigger_eval.rear_trigger));
}

void SimulationCore::stepUpdateContactEvents(StepCtx& ctx)
{
    CharacterState& ch = m_state.character;
    SimEvents&      events = m_state.events;
    events.left_touchdown  = !ctx.prev_contact_left && ch.foot_left.on_ground;
    events.right_touchdown = !ctx.prev_contact_right && ch.foot_right.on_ground;
    events.landed_from_jump = ctx.prev_jump_flight_active && !ch.jump_flight_active
                           && (events.left_touchdown || events.right_touchdown);
}

void SimulationCore::stepUpdateSlideEvents(double dt)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;
    SimEvents&      events = m_state.events;

    ch.left_slide_emit_timer = std::max(0.0, ch.left_slide_emit_timer - dt);
    ch.right_slide_emit_timer = std::max(0.0, ch.right_slide_emit_timer - dt);

    const bool left_slide_active_now = isSlideActive(ch.foot_left, cm.velocity.x);
    const bool right_slide_active_now = isSlideActive(ch.foot_right, cm.velocity.x);
    events.left_slide_active = left_slide_active_now && ch.left_slide_emit_timer <= 0.0;
    events.right_slide_active = right_slide_active_now && ch.right_slide_emit_timer <= 0.0;

    if (events.left_slide_active)
        ch.left_slide_emit_timer = m_config.particles.slide_emit_interval_s;
    if (events.right_slide_active)
        ch.right_slide_emit_timer = m_config.particles.slide_emit_interval_s;
    if (!left_slide_active_now)
        ch.left_slide_emit_timer = 0.0;
    if (!right_slide_active_now)
        ch.right_slide_emit_timer = 0.0;
}

bool SimulationCore::stepLaunchSwing(bool step_left, bool corrective, StepCtx& ctx)
{
    CMState&        cm = m_state.cm;
    CharacterState& ch = m_state.character;

    FootState& step_foot   = step_left ? ch.foot_left  : ch.foot_right;
    FootState& stance_foot = step_left ? ch.foot_right : ch.foot_left;
    if (step_foot.swinging) return false;

    double tx = computeStepLandingX(ctx.eff_walk, ch, stance_foot,
                                    ctx.pelvis, ctx.xi, ctx.L, ctx.reach_radius, ctx.ref_slope);
    if (ctx.landing_recovery_active) {
        FootState predicted = step_foot;
        predicted.swing_start  = step_foot.pos;
        predicted.swing_target = { tx, m_terrain.height_at(tx) };
        predicted.swing_t      = 0.0;
        refreshSwingArcProfile(predicted, m_terrain, ctx.L, ctx.eff_step, ctx.eff_walk,
                               ctx.speed_abs, ctx.max_spd);
        const double t_remaining = estimateSwingRemainingTime(predicted, ctx.eff_walk);
        const double future_cm_x = cm.position.x + cm.velocity.x * t_remaining;
        tx = retargetLandingRecoveryX(tx, ch, stance_foot, ctx.pelvis, future_cm_x,
                                      ctx.reach_radius, ctx.L, ctx.ref_slope, ctx.eff_walk,
                                      ctx.landing_recovery_gain);
    }
    beginSwingStep(step_foot, stance_foot, ch, tx, m_terrain,
                   corrective, ctx.L, ctx.eff_step, ctx.eff_walk, ctx.speed_abs, ctx.max_spd);
    return true;
}
