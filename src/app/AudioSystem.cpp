#include "app/AudioSystem.h"

#include <algorithm>
#include <cmath>

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

void AudioSystem::queueVariant(const PlaybackParams& params)
{
    if (m_device == 0 || m_footstep_sample.empty()) return;

    const float clamped_gain  = std::clamp(params.gain, 0.0f, 2.0f);
    const float clamped_pitch = std::clamp(params.pitch, 0.55f, 1.65f);
    const float start_ratio   = std::clamp(params.trim_start, 0.0f, 0.95f);
    const float end_ratio     = std::clamp(params.trim_end, start_ratio + 0.02f, 1.0f);

    const std::size_t src_size = m_footstep_sample.size();
    const std::size_t src_start = static_cast<std::size_t>(start_ratio * static_cast<float>(src_size - 1));
    const std::size_t src_end = std::max(src_start + 2,
        static_cast<std::size_t>(end_ratio * static_cast<float>(src_size)));
    const double trimmed_len = static_cast<double>(src_end - src_start);
    const std::size_t dst_size = std::max<std::size_t>(
        8, static_cast<std::size_t>(std::ceil(trimmed_len / clamped_pitch)));

    std::vector<float> scaled(dst_size, 0.0f);
    for (std::size_t i = 0; i < dst_size; ++i) {
        const double src_pos = static_cast<double>(src_start) + static_cast<double>(i) * clamped_pitch;
        const double src_clamped = std::clamp(src_pos, static_cast<double>(src_start), static_cast<double>(src_end - 1));
        const std::size_t i0 = static_cast<std::size_t>(src_clamped);
        const std::size_t i1 = std::min(i0 + 1, src_end - 1);
        const float frac = static_cast<float>(src_clamped - static_cast<double>(i0));
        const float sample = std::lerp(m_footstep_sample[i0], m_footstep_sample[i1], frac);

        const float edge = static_cast<float>(i) / static_cast<float>(std::max<std::size_t>(1, dst_size - 1));
        const float fade_in = std::min(1.0f, edge / 0.10f);
        const float fade_out = std::min(1.0f, (1.0f - edge) / 0.18f);
        scaled[i] = sample * clamped_gain * std::min(fade_in, fade_out);
    }

    SDL_QueueAudio(m_device, scaled.data(), static_cast<Uint32>(scaled.size() * sizeof(float)));
}

void AudioSystem::playTouchdown(bool left)
{
    const std::uint32_t v = m_variant_counter++;
    const float side_bias = left ? -1.0f : 1.0f;
    PlaybackParams params;
    params.gain = 0.56f + 0.10f * static_cast<float>(v % 3);
    params.pitch = 0.96f + 0.05f * side_bias + 0.04f * static_cast<float>((v / 2) % 3);
    params.trim_start = 0.01f;
    params.trim_end = 0.66f;
    queueVariant(params);
}

void AudioSystem::playLanding(float impact_gain)
{
    const std::uint32_t v = m_variant_counter++;
    PlaybackParams params;
    params.gain = std::clamp(0.90f + 0.28f * impact_gain + 0.05f * static_cast<float>(v % 2), 0.0f, 1.8f);
    params.pitch = 0.74f + 0.05f * static_cast<float>(v % 3);
    params.trim_start = 0.0f;
    params.trim_end = 0.92f;
    queueVariant(params);
}

void AudioSystem::playSlide(bool left)
{
    const std::uint32_t v = m_variant_counter++;
    const float side_bias = left ? 0.03f : -0.03f;
    PlaybackParams params;
    params.gain = 0.18f + 0.04f * static_cast<float>(v % 3);
    params.pitch = 1.18f + side_bias + 0.05f * static_cast<float>((v / 2) % 2);
    params.trim_start = 0.10f;
    params.trim_end = 0.34f;
    queueVariant(params);
}

void AudioSystem::shutdown()
{
    if (m_device == 0) return;
    SDL_ClearQueuedAudio(m_device);
    SDL_CloseAudioDevice(m_device);
    m_device = 0;
}
