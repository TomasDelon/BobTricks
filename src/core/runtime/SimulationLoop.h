#pragma once

#include <cstdint>
#include <functional>

/**
 * @brief Boucle de simulation fixe avec accumulateur.
 *
 * Cette classe reçoit un delta temps réel et décide combien de pas fixes du
 * noyau doivent être exécutés pendant une frame.
 */
class SimulationLoop
{
public:
    /** @brief Paramètres de la boucle fixe. */
    struct Config {
        /** @brief Pas fixe nominal de simulation. */
        double fixed_dt_s       = 1.0 / 60.0;
        /** @brief Delta temps réel maximal accepté pour une frame. */
        double max_frame_dt_s   = 0.25;
        /** @brief Taille maximale de l'accumulateur pour éviter les spirales. */
        double max_accumulator_s = 0.25;
    };

    SimulationLoop() = default;
    explicit SimulationLoop(const Config& config);

    /** @brief Réinitialise l'accumulateur et les compteurs. */
    void reset();

    /** @brief Force l'état pause/reprise. */
    void setPaused(bool paused);
    /** @brief Inverse l'état pause/reprise. */
    void togglePaused();
    /** @brief Indique si la boucle est actuellement en pause. */
    bool isPaused() const;

    /** @brief Demande l'exécution d'un nombre borné de pas en mode pause. */
    void requestSingleStep(std::uint32_t count = 1);
    /** @brief Réécrit le compteur total de pas exécutés. */
    void setTotalStepCount(std::uint64_t count);
    /** @brief Applique un facteur de temps global à la simulation. */
    void setTimeScale(double time_scale);

    /** @brief Retourne la valeur courante du pas fixe. */
    double        getFixedDt()          const;
    /** @brief Retourne le facteur de temps global appliqué. */
    double        getTimeScale()        const;
    /** @brief Retourne le nombre total de pas fixes exécutés. */
    std::uint64_t getTotalStepCount()   const;
    /** @brief Retourne le temps simulé accumulé à partir des pas exécutés. */
    double        getSimulationTime()   const;

    /**
     * @brief Exécute une frame et retourne le nombre de pas fixes produits.
     * @param real_dt_s Delta temps réel mesuré pour la frame courante.
     * @param step_fn   Callback invoqué une fois par pas fixe produit.
     * @return Nombre de pas fixes effectivement exécutés pendant la frame.
     */
    std::uint32_t runFrame(double real_dt_s, const std::function<void(double)>& step_fn);

private:
    Config        m_config;
    double        m_accumulator_s        = 0.0;
    double        m_time_scale           = 1.0;
    bool          m_paused               = false;
    std::uint32_t m_pending_single_steps = 0;
    std::uint64_t m_total_step_count     = 0;
};
