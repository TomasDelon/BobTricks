#pragma once

#include "core/simulation/SimState.h"
#include "core/simulation/InputFrame.h"
#include "core/terrain/Terrain.h"
#include "config/AppConfig.h"

/**
 * @brief Noyau autonome de physique et de locomotion.
 *
 * Cette classe possède l'état du centre de masse, l'état du personnage, le
 * terrain et le temps de simulation. Elle ne dépend d'aucune API SDL/ImGui.
 */
class SimulationCore
{
public:
    /** @brief Construit le noyau à partir d'une configuration vivante. */
    explicit SimulationCore(AppConfig& config);

    /** @brief Avance la simulation d'un pas fixe. */
    void step(double dt, const InputFrame& input);

    /** @brief Réinitialise la simulation à un état initial connu. */
    void reset(const ScenarioInit& init);

    /** @brief Recharge un instantané complet de simulation. */
    void loadState(const SimState& snap);

    /** @brief Régénère le terrain à partir de la configuration courante. */
    void regenerateTerrain();
    /** @brief Téléporte le centre de masse, utilisé notamment par le panel IP. */
    void teleportCM(double x, double vx);
    /** @brief Force la vitesse du centre de masse. */
    void setCMVelocity(Vec2 vel);
    /** @brief Épingle ou désépingle un pied. */
    void toggleFootPin(bool left);
    /** @brief Épingle ou désépingle une main. */
    void toggleHandPin(bool left);

    /** @brief Retourne l'état courant en lecture seule. */
    const SimState& state()   const { return m_state; }
    double          time()    const { return m_state.sim_time; }
    const Terrain&  terrain() const { return m_terrain; }

private:
    AppConfig& m_config;   // non-owning — Application's m_config
    Terrain    m_terrain;  // declared after m_config (stores ref to m_config.terrain)
    SimState   m_state;
};
