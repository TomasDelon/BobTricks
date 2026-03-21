#include "core/simulation/SimulationCore.h"

#include <cmath>
#include <cstdio>    // fprintf — no SDL dependency

#include "core/simulation/SimVerbosity.h"

#include "core/character/FootState.h"
#include "core/locomotion/StandingController.h"
#include "core/locomotion/BalanceComputer.h"
#include "core/locomotion/StepPlanner.h"
#include "core/physics/Geometry.h"
#include "core/simulation/WorldConstants.h"

// ── Construction ──────────────────────────────────────────────────────────────

SimulationCore::SimulationCore(AppConfig& config)
    : m_config(config)
    , m_terrain(m_config.terrain)   // Terrain stores ref to m_config.terrain
{
    m_terrain.generate();

    // Place CM at nominal standing height above terrain baseline.
    const double L = m_config.character.body_height_m / 5.0;
    m_state.cm.position = {
        0.0,
        computeNominalY(L, m_config.standing.d_pref, m_config.character.cm_pelvis_ratio)
    };
}

// ── reset / loadState / UI helpers ────────────────────────────────────────────

void SimulationCore::reset(const ScenarioInit& init)
{
    if (init.terrain_seed != 0) {
        m_config.terrain.seed = static_cast<int>(init.terrain_seed);
        m_terrain.generate();
    }

    m_state.cm.position     = init.cm_pos;
    m_state.cm.velocity     = init.cm_vel;
    m_state.cm.acceleration = {0.0, 0.0};

    // Clear character completely — feet_initialized = false forces re-bootstrap.
    m_state.character                      = CharacterState{};
    m_state.character.facing               = (init.cm_vel.x >= 0.0) ? 1.0 : -1.0;
    m_state.character.facing_vel           = init.cm_vel.x;
    m_state.character.last_heel_strike_t   = -1.0;

    m_state.sim_time = 0.0;
}

void SimulationCore::loadState(const SimState& snap)
{
    m_state = snap;
}

void SimulationCore::regenerateTerrain()
{
    m_terrain.generate();
}

void SimulationCore::teleportCM(double x, double vx)
{
    m_state.cm.position.x = x;
    m_state.cm.velocity.x = vx;
    // Feet stay at old position after a teleport → re-bootstrap next step().
    m_state.character.feet_initialized = false;
    m_state.character.step_plan        = {};
}

void SimulationCore::setCMVelocity(Vec2 vel)
{
    m_state.cm.velocity = vel;
}

// ── step ──────────────────────────────────────────────────────────────────────

void SimulationCore::step(double dt, const InputFrame& input)
{
    CMState&        cm        = m_state.cm;
    CharacterState& character = m_state.character;

    // ── Large-perturbation guard ──────────────────────────────────────────────
    // Must run BEFORE the bootstrap block so the re-bootstrap fires this frame.
    // If the CM drifted more than 2L from the nearest foot (max leg reach), the
    // feet are geometrically unreachable.  Flag for re-bootstrap rather than
    // queuing many sequential steps that take seconds to catch up (floating look).
    if (character.feet_initialized) {
        const double L_guard   = m_config.character.body_height_m / 5.0;
        const double near_dist = std::min(
            std::abs(m_state.cm.position.x - character.foot_left.pos.x),
            std::abs(m_state.cm.position.x - character.foot_right.pos.x));
        if (near_dist > 2.0 * L_guard) {
            if (g_sim_verbose)
                fprintf(stderr, "[Bootstrap] re-bootstrap: near_dist=%.3f > 2L=%.3f\n",
                        near_dist, 2.0 * L_guard);
            character.step_plan        = {};
            character.feet_initialized = false;
        }
    }

    // ── Bootstrap feet — first frame only (also re-runs after guard above) ──
    if (!character.feet_initialized) {
        const double L        = m_config.character.body_height_m / 5.0;
        const double foot_sep = m_config.standing.d_pref * L;
        // At init, facing_vel=0 → lean=0 → pelvis_x = cm.x exactly.
        const double pelvis_x = cm.position.x;

        const double fx_L = pelvis_x - foot_sep * 0.5;
        const double fx_R = pelvis_x + foot_sep * 0.5;

        character.foot_left.pos      = { fx_L, m_terrain.height_at(fx_L) };
        character.foot_left.ground_y = character.foot_left.pos.y;
        character.foot_left.phase    = FootPhase::Planted;

        character.foot_right.pos      = { fx_R, m_terrain.height_at(fx_R) };
        character.foot_right.ground_y = character.foot_right.pos.y;
        character.foot_right.phase    = FootPhase::Planted;

        if (g_sim_verbose)
            fprintf(stderr, "[Bootstrap] L=%.3f  foot_sep=%.3f  pelvis_x=%.3f  "
                    "foot_L=(%.3f, %.3f)  foot_R=(%.3f, %.3f)\n",
                    L, foot_sep, pelvis_x,
                    character.foot_left.pos.x,  character.foot_left.pos.y,
                    character.foot_right.pos.x, character.foot_right.pos.y);

        character.feet_initialized = true;

        // Derive support immediately so target_y is correct on this first frame.
        updateSupportState(character.support,
                           character.foot_left, character.foot_right);
    }

    // ── Physics setup ─────────────────────────────────────────────────────────
    const PhysicsConfig& ph = m_config.physics;
    const double L = m_config.character.body_height_m / 5.0;


    // Smoothed ground ref: sample one limb-length ahead and behind, average.
    // Avoids discontinuities when CM crosses a terrain vertex.
    const double h_back    = m_terrain.height_at(cm.position.x - L);
    const double h_fwd     = m_terrain.height_at(cm.position.x + L);
    const double ref_h     = World::GROUND_Y + (h_back + h_fwd) * 0.5;
    const double terrain_h = World::GROUND_Y + m_terrain.height_at(cm.position.x);

    // Nominal CM height above terrain using geometry-corrected formula.
    const double nominal_y = ref_h
                           + computeNominalY(L,
                                             m_config.standing.d_pref,
                                             m_config.character.cm_pelvis_ratio);

    // CM bob during swing: raises the spring target at mid-swing (φ=0.5).
    double spring_target_y = nominal_y;
    if (character.feet_initialized && character.step_plan.active) {
        const StepPlan& plan = character.step_plan;
        const double phi = std::max(0.0, std::min(1.0,
            (m_state.sim_time - plan.t_start) / plan.duration));
        const double A = m_config.step.k_bob * L
                       * std::tanh(std::abs(cm.velocity.x) / m_config.step.v_ref_bob);
        spring_target_y = nominal_y + A * std::sin(M_PI * phi);
    }

    const bool on_floor = (cm.position.y <= spring_target_y + 1e-3);

    // ── Forces ────────────────────────────────────────────────────────────────
    Vec2 accel = {0.0, 0.0};

    if (ph.gravity_enabled)
        accel.y -= ph.gravity;

    if (ph.spring_enabled && on_floor) {
        accel.y += ph.spring_stiffness * (spring_target_y - cm.position.y)
                 - ph.spring_damping   *  cm.velocity.y;
    }

    if (on_floor) {
        if (input.key_right) accel.x += ph.move_force;
        if (input.key_left)  accel.x -= ph.move_force;
    }

    // IP completion force: drives CM past midstance after key release.
    // Guards: offset>0 (CM past midstance), |vx|>eps_v, capped at move_force.
    if (on_floor && character.step_plan.active && !input.key_left && !input.key_right
            && std::abs(cm.velocity.x) > m_config.standing.eps_v) {
        const StepPlan&  plan   = character.step_plan;
        const FootState& stance = plan.move_right ? character.foot_left
                                                  : character.foot_right;
        const double offset = (cm.position.x - stance.pos.x) * character.facing;
        if (offset > 0.0) {
            const double ax_raw = (ph.gravity / nominal_y) * offset;
            accel.x += character.facing * std::min(ax_raw, ph.move_force);
        }
    }

    // Standing balance correction: gentle restoring force toward support center.
    // Coefficient 0.5·ω₀² cancels half the IP instability → stable quiet standing.
    if (on_floor
            && character.support.both_planted()
            && !character.step_plan.active
            && !input.key_left && !input.key_right) {
        const double k_restore = 0.5 * (ph.gravity / nominal_y);
        accel.x += k_restore * (character.support.center() - cm.position.x);
    }

    if (ph.air_friction_enabled) {
        accel.x -= ph.air_friction * cm.velocity.x;
        accel.y -= ph.air_friction * cm.velocity.y;
    }

    cm.acceleration = accel;

    // ── Integration ───────────────────────────────────────────────────────────
    cm.velocity += accel * dt;
    cm.position += cm.velocity * dt;

    // Jump impulse — one-shot, caller clears InputFrame.jump after delivery.
    if (input.jump && on_floor)
        cm.velocity.y = ph.jump_impulse;

    // Velocity override — scripted/headless scenarios and drag-set in UI.
    if (input.set_velocity)
        cm.velocity = *input.set_velocity;

    // Floor friction — multiplicative: vx *= exp(-mu·dt), never reverses sign.
    if (ph.floor_friction_enabled && on_floor)
        cm.velocity.x *= std::exp(-ph.floor_friction * dt);

    // Hard terrain clamp.
    if (cm.position.y < terrain_h) {
        cm.position.y = terrain_h;
        if (cm.velocity.y < 0.0) cm.velocity.y = 0.0;
    }

    // ── Character pose ────────────────────────────────────────────────────────
    const bool on_floor_post = (cm.position.y <= spring_target_y + 1e-3);
    updateCharacterState(character, cm,
                         m_config.character, m_config.reconstruction,
                         on_floor_post, dt);

    updateSupportState(character.support, character.foot_left, character.foot_right);

    if (character.feet_initialized)
        character.balance = computeBalanceState(cm, character.support,
                                                m_config.character, m_config.physics);

    // ── Locomotion FSM ────────────────────────────────────────────────────────
    // Simple two-state: Walking (step active) vs Standing (both planted + slow).
    // No Falling gate — step planner always runs its own recovery.
    if (character.feet_initialized
            && character.locomotion_state != LocomotionState::Airborne) {
        if (character.step_plan.active) {
            character.locomotion_state = LocomotionState::Walking;
        } else if (character.support.both_planted()
                   && std::abs(cm.velocity.x) <= m_config.standing.eps_v) {
            character.locomotion_state = LocomotionState::Standing;
        }
    }

    // ── Step planner ──────────────────────────────────────────────────────────
    if (character.feet_initialized) {
        character.last_trigger      = StepTriggerType::None;  // reset — set below if trigger fires
        character.heel_strike_this_tick = false;               // reset — set below on landing

        StepPlan&    plan  = character.step_plan;
        const double sim_t = m_state.sim_time;

        // 1. Update swing foot position.
        if (plan.active) {
            FootState& swing = plan.move_right ? character.foot_right
                                               : character.foot_left;
            swing.pos = evalSwingFoot(plan, swing, m_terrain, sim_t);

            // 2. Heel-strike: u >= 1.
            const double u = (sim_t - plan.t_start) / plan.duration;
            if (u >= 1.0) {
                swing.pos      = plan.land_target;
                swing.ground_y = m_terrain.height_at(swing.pos.x);
                swing.phase    = FootPhase::Planted;
                plan.active    = false;
                character.last_heel_strike_t    = sim_t;
                character.heel_strike_this_tick = true;

                if (g_sim_verbose)
                    fprintf(stderr, "[StepPlanner] heel-strike  foot=(%s)  pos=(%.3f, %.3f)\n",
                            plan.move_right ? "R" : "L", swing.pos.x, swing.pos.y);

                updateSupportState(character.support,
                                   character.foot_left, character.foot_right);
                character.balance = computeBalanceState(cm, character.support,
                                                        m_config.character, m_config.physics);
            }
        }

        // 3. Check trigger and launch new step.
        if (!plan.active) {
            bool swing_right          = false;
            bool emergency            = false;
            StepTriggerType trigger   = StepTriggerType::None;

            // Friction-based stopping distance: where the CM actually comes to rest
            // with both feet planted (floor friction + restoring force active).
            // Uses μ = floor_friction (s⁻¹); clamped away from zero for safety.
            const double cm_reach = cm.position.x
                                  + cm.velocity.x
                                  / std::max(0.1, m_config.physics.floor_friction);

            const bool fired = shouldStep(character.pelvis,
                                          character.foot_left, character.foot_right,
                                          cm, cm_reach,
                                          plan,
                                          m_config.standing, m_config.walk,
                                          L, character.facing,
                                          &swing_right, &emergency, &trigger);
            if (fired) {
                character.last_trigger = trigger;   // record intent regardless of plan success
                const FootState& foot_swing  = swing_right ? character.foot_right
                                                           : character.foot_left;
                const FootState& foot_stance = swing_right ? character.foot_left
                                                           : character.foot_right;
                plan = planStep(swing_right, emergency,
                                foot_swing, foot_stance,
                                cm, m_config.step, m_config.standing,
                                m_config.walk, m_terrain,
                                sim_t, L, character.facing,
                                character.pelvis);
                if (plan.active) {
                    FootState& swing = plan.move_right ? character.foot_right
                                                      : character.foot_left;
                    swing.phase = FootPhase::Swing;
                } else {
                    if (g_sim_verbose)
                        fprintf(stderr, "[StepPlanner] infeasible, skip\n");
                }
            }
        }
    }

    // Advance time at end of step (so sim_t used above reflects the step start).
    m_state.sim_time += dt;
}
