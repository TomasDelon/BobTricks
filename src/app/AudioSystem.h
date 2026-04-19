#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

class AudioSystem
{
public:
    bool init();
    bool loadFootstepSample(const char* path);
    void playFootstep(float gain);
    void shutdown();

private:
    SDL_AudioDeviceID m_device = 0;
    std::vector<float> m_footstep_sample;
};
