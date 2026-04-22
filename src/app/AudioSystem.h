#pragma once

/**
 * @file AudioSystem.h
 * @brief Système audio réactif aux événements de locomotion.
 */

#include <string>

#include <SDL2/SDL_mixer.h>

#include "config/AppConfig.h"

/**
 * @brief Système audio piloté par les événements de contact pied-sol.
 *
 * Ce système repose sur SDL_mixer pour jouer un sample de pas et une piste
 * musicale bouclée. Il est déclenché par `Application` en réponse aux
 * événements `SimEvents::left_touchdown`, `right_touchdown` et
 * `landed_from_jump`.
 */
class AudioSystem
{
public:
    /**
     * @brief Initialise le périphérique audio SDL2.
     * @return Vrai si l'initialisation a réussi.
     */
    bool init();

    /**
     * @brief Charge le sample de pas depuis un fichier WAV mono float32.
     * @param path Chemin vers le fichier WAV.
     * @return Vrai si le chargement a réussi.
     */
    bool loadFootstepSample(const char* path);

    /** @brief Applique les réglages runtime (volumes, piste active, activation musique). */
    void applyConfig(const AudioConfig& config);

    /**
     * @brief Joue le son de contact d'un pied.
     * @param left Vrai pour le pied gauche, faux pour le droit.
     */
    void playTouchdown(bool left);

    /**
     * @brief Joue le son d'atterrissage après un saut.
     * @param impact_gain Gain additionnel proportionnel à la vitesse d'impact.
     */
    void playLanding(float impact_gain = 1.0f);

    /**
     * @brief Joue le son de glissement d'un pied sur le terrain.
     * @param left Vrai pour le pied gauche.
     */
    void playSlide(bool left);

    /** @brief Libère le périphérique audio SDL2. */
    void shutdown();

    /** @brief Nombre de pistes musicales trouvées dans `data/audio/music/`. */
    static int musicTrackCount();
    /** @brief Libellé d'une piste musicale pour l'interface de debug. */
    static const char* musicTrackLabel(int index);

private:
    void playVariant(float gain);
    bool loadMusicTrack(int track_index);
    void refreshMusicPlayback();

    Mix_Chunk*  m_footstep_sample = nullptr;
    Mix_Music*  m_music           = nullptr;
    AudioConfig m_runtime_config;
};
