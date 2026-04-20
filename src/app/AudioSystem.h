#pragma once

#include <SDL2/SDL.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class AudioSystem
{
public:
    bool init();
    bool loadFootstepSample(const char* path);
    void playTouchdown(bool left);
    void playLanding(float impact_gain = 1.0f);
    void playSlide(bool left);
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
