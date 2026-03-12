#pragma once

#include "core/debug/DebugSnapshot.hpp"

namespace bobtricks {

/**
 * \brief Backend minimal de debug partage par les futures interfaces.
 */
class DebugCommandBus {
public:
    /**
     * \brief Bascule l'etat pause du runtime.
     */
    void togglePause();

    /**
     * \brief Augmente l'echelle temporelle.
     */
    void speedUp();

    /**
     * \brief Reduit l'echelle temporelle.
     */
    void slowDown();

    /**
     * \brief Retourne l'echelle temporelle courante.
     */
    double getTimeScale() const;

    /**
     * \brief Indique si la simulation est en pause.
     */
    bool isPaused() const;

    /**
     * \brief Publie un snapshot minimal du debug.
     */
    DebugSnapshot createSnapshot() const;

private:
    bool paused_ {false};
    double timeScale_ {1.0};
};

}  // namespace bobtricks
