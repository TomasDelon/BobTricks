#pragma once

#include <cstdint>
#include <functional>

/**
 * @file SimulationLoop.h
 * @brief Boucle de simulation à pas fixe avec accumulateur de temps.
 */

/**
 * @brief Boucle de simulation fixe avec accumulateur.
 *
 * Cette classe reçoit un delta temps réel et décide combien de pas fixes du
 * noyau doivent être exécutés pendant une frame. Elle gère la pause, le
 * step-by-step, l'échelle de temps et le dépassement de frame (`spiral of death`
 * évité par `max_accumulator_s`).
 */
class SimulationLoop
{
public:
    /** @brief Paramètres de la boucle fixe. */
    struct Config {
        double fixed_dt_s       = 1.0 / 60.0;
        double max_frame_dt_s   = 0.25;
        double max_accumulator_s = 0.25;
    };

    SimulationLoop() = default;
    explicit SimulationLoop(const Config& config);

    /** @brief Réinitialise l'accumulateur et les compteurs. */
    void reset();

    /** @brief Met en pause ou reprend la simulation. */
    void setPaused(bool paused);
    /** @brief Bascule l'état de pause. */
    void togglePaused();
    /** @brief Retourne vrai si la simulation est en pause. */
    bool isPaused() const;

    /** @brief Demande l'exécution de `count` pas fixes en mode pause (step-by-step). */
    void requestSingleStep(std::uint32_t count = 1);
    /** @brief Injecte un compteur de pas initial (utilisé lors du chargement d'un snapshot). */
    void setTotalStepCount(std::uint64_t count);
    /** @brief Ajuste l'échelle de temps (1.0 = temps réel, 0.5 = ralenti 2×). */
    void setTimeScale(double time_scale);

    /** @brief Pas de temps fixe de la simulation (s). */
    double        getFixedDt()          const;
    /** @brief Échelle de temps courante. */
    double        getTimeScale()        const;
    /** @brief Nombre total de pas fixes exécutés depuis le démarrage. */
    std::uint64_t getTotalStepCount()   const;
    /** @brief Temps de simulation cumulé en secondes. */
    double        getSimulationTime()   const;

    /**
     * @brief Exécute une frame et retourne le nombre de pas fixes produits.
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
