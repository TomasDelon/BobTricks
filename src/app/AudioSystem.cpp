#include "app/AudioSystem.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <cctype>
#include <filesystem>
#include <string>
#include <vector>

namespace {

struct MusicTrack {
    std::string label;
    std::string path;
};

const std::vector<MusicTrack>& musicTracks()
{
    static const std::vector<MusicTrack> tracks = [] {
        namespace fs = std::filesystem;

        std::vector<MusicTrack> out;
        const fs::path music_dir = "data/audio/music";
        if (!fs::exists(music_dir) || !fs::is_directory(music_dir))
            return out;

        std::vector<fs::path> paths;
        for (const fs::directory_entry& entry : fs::directory_iterator(music_dir)) {
            if (!entry.is_regular_file())
                continue;

            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (ext == ".mp3" || ext == ".ogg" || ext == ".wav")
                paths.push_back(entry.path());
        }

        std::sort(paths.begin(), paths.end());
        for (const fs::path& path : paths) {
            out.push_back({
                path.stem().string(),
                path.string(),
            });
        }
        return out;
    }();

    return tracks;
}

int clampMixerVolume(double gain)
{
    return std::clamp(static_cast<int>(gain * static_cast<double>(MIX_MAX_VOLUME)),
                      0, MIX_MAX_VOLUME);
}

int footstepGainToMixer(double gain)
{
    const double normalized = std::clamp(gain / 4.0, 0.0, 1.0);
    const double curved = std::pow(normalized, 0.55);
    return clampMixerVolume(curved);
}

int musicGainToMixer(double gain)
{
    const double normalized = std::clamp(gain, 0.0, 1.0);
    const double curved = std::pow(normalized, 2.6);
    return clampMixerVolume(curved);
}

void normalizeChunkPeak(Mix_Chunk* chunk)
{
    if (!chunk || !chunk->abuf || chunk->alen == 0)
        return;

    int freq = 0;
    Uint16 format = 0;
    int channels = 0;
    if (Mix_QuerySpec(&freq, &format, &channels) == 0)
        return;

    if (format != AUDIO_F32SYS)
        return;

    float* samples = reinterpret_cast<float*>(chunk->abuf);
    const std::size_t sample_count = static_cast<std::size_t>(chunk->alen) / sizeof(float);
    float peak = 0.0f;
    for (std::size_t i = 0; i < sample_count; ++i)
        peak = std::max(peak, std::abs(samples[i]));

    if (peak < 1.0e-5f)
        return;

    const float target_peak = 0.95f;
    const float scale = target_peak / peak;
    for (std::size_t i = 0; i < sample_count; ++i)
        samples[i] *= scale;
}

} // namespace

int AudioSystem::musicTrackCount()
{
    return static_cast<int>(musicTracks().size());
}

const char* AudioSystem::musicTrackLabel(int index)
{
    const auto& tracks = musicTracks();
    if (index < 0 || index >= musicTrackCount())
        return "Inconnue";
    return tracks[static_cast<std::size_t>(index)].label.c_str();
}

bool AudioSystem::init()
{
    const int init_flags = MIX_INIT_MP3;
    const int got_flags = Mix_Init(init_flags);
    if ((got_flags & init_flags) != init_flags) {
        SDL_Log("Mix_Init failed: %s", Mix_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, AUDIO_F32SYS, 2, 2048) < 0) {
        SDL_Log("Mix_OpenAudio failed: %s", Mix_GetError());
        Mix_Quit();
        return false;
    }

    Mix_AllocateChannels(16);
    return true;
}

bool AudioSystem::loadFootstepSample(const char* path)
{
    if (m_footstep_sample) {
        Mix_FreeChunk(m_footstep_sample);
        m_footstep_sample = nullptr;
    }

    m_footstep_sample = Mix_LoadWAV(path);
    if (!m_footstep_sample) {
        SDL_Log("Mix_LoadWAV failed for %s: %s", path, Mix_GetError());
        return false;
    }

    normalizeChunkPeak(m_footstep_sample);
    Mix_VolumeChunk(m_footstep_sample, footstepGainToMixer(m_runtime_config.footstep_volume));
    return true;
}

bool AudioSystem::loadMusicTrack(int track_index)
{
    const auto& tracks = musicTracks();

    if (m_music) {
        Mix_HaltMusic();
        Mix_FreeMusic(m_music);
        m_music = nullptr;
    }

    if (track_index < 0 || track_index >= musicTrackCount())
        return false;

    const std::string& path = tracks[static_cast<std::size_t>(track_index)].path;
    m_music = Mix_LoadMUS(path.c_str());
    if (!m_music) {
        SDL_Log("Mix_LoadMUS failed for %s: %s", path.c_str(), Mix_GetError());
        return false;
    }
    return true;
}

void AudioSystem::refreshMusicPlayback()
{
    Mix_VolumeMusic(musicGainToMixer(m_runtime_config.music_volume));

    if (!m_runtime_config.music_enabled) {
        Mix_HaltMusic();
        return;
    }

    if (!m_music && !loadMusicTrack(m_runtime_config.music_track))
        return;

    if (!Mix_PlayingMusic()) {
        if (Mix_PlayMusic(m_music, -1) < 0)
            SDL_Log("Mix_PlayMusic failed: %s", Mix_GetError());
    }
}

void AudioSystem::applyConfig(const AudioConfig& config)
{
    const bool track_changed = config.music_track != m_runtime_config.music_track;
    const bool music_state_changed = config.music_enabled != m_runtime_config.music_enabled;
    m_runtime_config = config;

    if (m_footstep_sample)
        Mix_VolumeChunk(m_footstep_sample, footstepGainToMixer(m_runtime_config.footstep_volume));

    if (track_changed)
        loadMusicTrack(m_runtime_config.music_track);

    if (track_changed || music_state_changed || Mix_PlayingMusic() == 0)
        refreshMusicPlayback();
    else
        Mix_VolumeMusic(musicGainToMixer(m_runtime_config.music_volume));
}

void AudioSystem::playVariant(float gain)
{
    if (!m_footstep_sample)
        return;

    const int channel = Mix_PlayChannel(-1, m_footstep_sample, 0);
    if (channel < 0)
        return;

    const double total_gain = gain * m_runtime_config.footstep_volume;
    Mix_Volume(channel, footstepGainToMixer(total_gain));
}

void AudioSystem::playTouchdown(bool left)
{
    playVariant(left ? 0.98f : 1.08f);
}

void AudioSystem::playLanding(float impact_gain)
{
    const float clamped_impact = std::clamp(impact_gain, 0.6f, 1.8f);
    playVariant(1.25f * clamped_impact);
}

void AudioSystem::playSlide(bool left)
{
    playVariant(left ? 0.28f : 0.32f);
}

void AudioSystem::shutdown()
{
    Mix_HaltMusic();

    if (m_music) {
        Mix_FreeMusic(m_music);
        m_music = nullptr;
    }
    if (m_footstep_sample) {
        Mix_FreeChunk(m_footstep_sample);
        m_footstep_sample = nullptr;
    }

    if (Mix_QuerySpec(nullptr, nullptr, nullptr) != 0)
        Mix_CloseAudio();
    Mix_Quit();
}
