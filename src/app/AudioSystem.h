#pragma once

/**
 * @file AudioSystem.h
 * @brief Système audio réactif aux événements de locomotion.
 */

#include <SDL2/SDL.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Système audio piloté par les événements de contact pied-sol.
 *
 * Ce système utilise l'API audio SDL2 bas niveau pour jouer des variantes
 * d'un sample de pas unique avec pitch et gain aléatoires. Il est déclenché
 * par `Application` en réponse aux événements `SimEvents::left_touchdown`,
 * `right_touchdown` et `landed_from_jump`.
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

private:
    struct PlaybackParams {
        float gain       = 1.0f;
        float pitch      = 1.0f;
        float trim_start = 0.0f;
        float trim_end   = 1.0f;
    };

    void queueVariant(const PlaybackParams& params);

    SDL_AudioDeviceID m_device = 0;
    std::vector<float> m_footstep_sample;
    std::uint32_t      m_variant_counter = 0;
};
