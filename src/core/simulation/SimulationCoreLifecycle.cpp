#include "core/simulation/SimulationCore.h"

#include "core/character/HeadController.h"

SimulationCore::SimulationCore(AppConfig& config)
    : m_config(config), m_terrain(config.terrain)
{
    m_terrain.generate();
}

void SimulationCore::reset(const ScenarioInit& init)
{
    if (init.terrain_seed != 0) {
        m_config.terrain.seed = static_cast<int>(init.terrain_seed);
        m_terrain.generate();
    }
    m_state             = SimState{};
    m_state.cm.position = init.cm_pos;
    m_state.cm.velocity = init.cm_vel;
    resetHeadState(m_state.character);
}

void SimulationCore::loadState(const SimState& snap) { m_state = snap; }

void SimulationCore::regenerateTerrain()
{
    m_terrain.generate();
    m_state.character.feet_initialized = false;
    m_state.character.ground_reference_initialized = false;
}

void SimulationCore::teleportCM(double x, double vx)
{
    m_state.cm.position.x = x;
    m_state.cm.velocity.x = vx;
    m_state.character.ground_reference_initialized = false;
    resetHeadState(m_state.character);
}

void SimulationCore::setCMVelocity(const Vec2& vel) { m_state.cm.velocity = vel; }

const SimState& SimulationCore::state() const
{
    return m_state;
}

double SimulationCore::time() const
{
    return m_state.sim_time;
}

const Terrain& SimulationCore::terrain() const
{
    return m_terrain;
}

void SimulationCore::toggleFootPin(bool left)
{
    FootState& foot = left ? m_state.character.foot_left : m_state.character.foot_right;
    foot.pinned = !foot.pinned;
    if (foot.pinned) {
        foot.pinned_pos    = foot.pos;
        foot.pinned_normal = foot.ground_normal;
    }
}

void SimulationCore::toggleHandPin(bool left)
{
    CharacterState& ch = m_state.character;
    bool& pinned = left ? ch.hand_left_pinned : ch.hand_right_pinned;
    Vec2& target = left ? ch.hand_left_target : ch.hand_right_target;
    const Vec2 current = left ? ch.hand_left : ch.hand_right;

    pinned = !pinned;
    if (pinned)
        target = current;
}

void SimulationCore::step(double dt, const InputFrame& input)
{
    m_state.sim_time += dt;
    StepCtx ctx;
    stepComputeConstants(ctx);
    stepBootstrapCM(ctx, input);
    stepGroundReference(ctx, dt);
    stepProcessInput(ctx, input);
    stepBlendRunMode(ctx, input, dt);
    stepBlendParams(ctx);
    stepIntegratePhysics(ctx, dt);
    stepReconstructGeometry(ctx, input);
    stepAdvanceSwing(ctx, dt);
    stepComputeTriggerState(ctx);
    stepAirborneJump(ctx);
    stepFireTriggers(ctx);
    stepApplyConstraints(ctx);
    stepWriteOutput(ctx, input, dt);
}
