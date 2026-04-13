#pragma once

namespace bobtricks {

/**
 * \brief Boucle a pas fixe 60 Hz avec accumulateur.
 */
class SimulationLoop {
public:
    static constexpr double kFixedStepSeconds = 1.0 / 60.0;
    static constexpr int kMaxStepsPerFrame = 5;

    /**
     * \brief Ajoute le temps reel ecoule a l'accumulateur.
     */
    void addFrameTime(double realDeltaSeconds, double timeScale, bool paused);

    /**
     * \brief Indique si un pas fixe doit etre execute.
     */
    bool shouldRunStep() const;

    /**
     * \brief Consomme un pas fixe de l'accumulateur.
     */
    void consumeStep();

private:
    double accumulatorSeconds_ {0.0};
    int stepsRunThisFrame_ {0};
};

}  // namespace bobtricks
