#include "app/AudioSystem.h"

#include <algorithm>

bool AudioSystem::init()
{
    SDL_AudioSpec desired{};
    desired.freq = 44100;
    desired.format = AUDIO_F32SYS;
    desired.channels = 1;
    desired.samples = 2048;

    SDL_AudioSpec obtained{};
    m_device = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (m_device == 0)
        return false;

    if (obtained.format != AUDIO_F32SYS || obtained.channels != 1) {
        SDL_Log("Unexpected audio format obtained; closing device");
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
        return false;
    }

    SDL_PauseAudioDevice(m_device, 0);
    return true;
}

bool AudioSystem::loadFootstepSample(const char* path)
{
    SDL_AudioSpec wav_spec{};
    Uint8* wav_buffer = nullptr;
    Uint32 wav_length = 0;
    if (SDL_LoadWAV(path, &wav_spec, &wav_buffer, &wav_length) == nullptr) {
        SDL_Log("SDL_LoadWAV failed for %s: %s", path, SDL_GetError());
        return false;
    }

    SDL_AudioCVT cvt{};
    if (SDL_BuildAudioCVT(&cvt,
                          wav_spec.format, wav_spec.channels, wav_spec.freq,
                          AUDIO_F32SYS, 1, 44100) < 0) {
        SDL_Log("SDL_BuildAudioCVT failed: %s", SDL_GetError());
        SDL_FreeWAV(wav_buffer);
        return false;
    }

    cvt.len = static_cast<int>(wav_length);
    cvt.buf = static_cast<Uint8*>(SDL_malloc(static_cast<std::size_t>(cvt.len) * cvt.len_mult));
    if (!cvt.buf) {
        SDL_Log("SDL_malloc failed for audio conversion");
        SDL_FreeWAV(wav_buffer);
        return false;
    }
    SDL_memcpy(cvt.buf, wav_buffer, wav_length);
    SDL_FreeWAV(wav_buffer);

    if (SDL_ConvertAudio(&cvt) < 0) {
        SDL_Log("SDL_ConvertAudio failed: %s", SDL_GetError());
        SDL_free(cvt.buf);
        return false;
    }

    const std::size_t sample_count = static_cast<std::size_t>(cvt.len_cvt) / sizeof(float);
    const float* sample_data = reinterpret_cast<const float*>(cvt.buf);
    m_footstep_sample.assign(sample_data, sample_data + sample_count);
    SDL_free(cvt.buf);
    return !m_footstep_sample.empty();
}

void AudioSystem::playFootstep(float gain)
{
    if (m_device == 0 || m_footstep_sample.empty()) return;

    const float clamped_gain = std::clamp(gain, 0.0f, 2.0f);
    std::vector<float> scaled = m_footstep_sample;
    for (float& sample : scaled)
        sample *= clamped_gain;

    SDL_QueueAudio(m_device, scaled.data(),
                   static_cast<Uint32>(scaled.size() * sizeof(float)));
}

void AudioSystem::shutdown()
{
    if (m_device == 0) return;
    SDL_ClearQueuedAudio(m_device);
    SDL_CloseAudioDevice(m_device);
    m_device = 0;
}
