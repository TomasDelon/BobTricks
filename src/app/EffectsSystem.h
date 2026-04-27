#pragma once

/**
 * @file EffectsSystem.h
 * @brief Système de particules visuelles réactif aux événements de locomotion.
 */

#include <deque>

#include "config/AppConfig.h"
#include "core/simulation/SimState.h"
#include "render/DustParticle.h"

/**
 * @brief Système de particules de poussière piloté par les événements de simulation.
 *
 * Ce système maintient un pool de `DustParticle` dans un `std::deque`. Il est
 * mis à jour à chaque pas de simulation par `Application` et consomme les
 * événements `SimEvents` pour émettre des rafales au toucher-sol et au
 * glissement. Les particules expirées sont supprimées à chaque appel à `update()`.
 */
class EffectsSystem
{
public:
    /**
     * @brief Met à jour les particules existantes et émet de nouvelles particules.
     *
     * @param state    État de simulation courant (lire les événements de contact).
     * @param char_cfg Paramètres morphologiques du personnage.
     * @param config   Paramètres de configuration des particules.
     * @param sim_time Temps de simulation courant (s).
     */
    void update(const SimState& state,
                const CharacterConfig& char_cfg,
                const ParticlesConfig& config,
                double sim_time);

    /** @brief Supprime toutes les particules actives. */
    void clear();

    /** @brief Retourne le pool de particules actives (pour rendu par `SceneRenderer`). */
    const std::deque<DustParticle>& dustParticles() const;

private:
    void emitFootDust(const FootState& foot,
                      const CharacterConfig& char_cfg,
                      const ParticlesConfig& config,
                      double sim_time,
                      double burst_scale,
                      double tangent_bias,
                      double vertical_scale);
    void emitSlideDust(const FootState& foot,
                       const CharacterConfig& char_cfg,
                       const ParticlesConfig& config,
                       double sim_time,
                       double tangent_dir);
    void pruneDustParticles(double sim_time);

    std::deque<DustParticle> m_dust_particles;
};
