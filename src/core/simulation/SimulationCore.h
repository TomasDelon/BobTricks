#pragma once

#include "core/simulation/SimState.h"
#include "core/simulation/InputFrame.h"
#include "core/terrain/Terrain.h"
#include "config/AppConfig.h"

// Self-contained physics + locomotion core.
//
// Owns:  CMState, CharacterState, Terrain, simulation time.
// Reads: AppConfig& (non-owning reference — caller owns and may mutate it live).
//
// Does NOT include SDL, ImGui, or any rendering header.
// Compiles and links without SDL:
//   g++ -std=c++20 src/core/simulation/SimulationCore.cpp [...core deps...] -Isrc -lm
class SimulationCore
{
public:
    // config must outlive SimulationCore (Application owns it).
    explicit SimulationCore(AppConfig& config);

    // Advance by one fixed step. InputFrame carries player / scripted input.
    void step(double dt, const InputFrame& input);

    // Reset to known initial conditions.  Terrain is regenerated if seed changes.
    // Feet are re-bootstrapped on the first step() after reset().
    void reset(const ScenarioInit& init);

    // Restore a full state snapshot — used by Application step-back.
    void loadState(const SimState& snap);

    // UI helpers — not for headless use.
    void regenerateTerrain();
    void teleportCM(double x, double vx);   // IP-test panel
    void setCMVelocity(Vec2 vel);            // drag-set-velocity

    // Read-only accessors.
    const SimState& state()   const { return m_state; }
    double          time()    const { return m_state.sim_time; }
    const Terrain&  terrain() const { return m_terrain; }

private:
    AppConfig& m_config;   // non-owning — Application's m_config
    Terrain    m_terrain;  // declared after m_config (stores ref to m_config.terrain)
    SimState   m_state;
};
