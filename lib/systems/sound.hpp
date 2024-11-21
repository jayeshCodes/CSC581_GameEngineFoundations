//
// Created by Jayesh Gajbhar on 11/21/24.
//

#ifndef SOUND_HPP
#define SOUND_HPP

#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "../ECS/system.hpp"
#include "../ECS/coordinator.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;

class SoundSystem : public System {
private:
    static constexpr int FREQUENCY = 44100;
    static constexpr int CHUNK_SIZE = 2048;
    static constexpr int CHANNELS = 16;

    std::unordered_map<std::string, Mix_Chunk *> soundEffects;
    std::unordered_map<std::string, Mix_Music *> musicTracks;

    float masterVolume = 1.0f;
    float musicVolume = 1.0f;
    float sfxVolume = 1.0f;

public:
    SoundSystem() {
        if (Mix_OpenAudio(FREQUENCY, MIX_DEFAULT_FORMAT, 2, CHUNK_SIZE) < 0) {
            std::cerr << "Error initializing SDL_mixer: " << Mix_GetError() << std::endl;
            return;
        }
        Mix_AllocateChannels(CHANNELS);
    }

    ~SoundSystem() {
        // Free all sound effects
        for (auto &[name, sound]: soundEffects) {
            if (sound) Mix_FreeChunk(sound);
        }
        for (auto &[name, music]: musicTracks) {
            if (music) Mix_FreeMusic(music);
        }
        Mix_CloseAudio();
    }

    // Load a sound effect
    bool loadSound(const std::string &name, const std::string &path) {
        if (soundEffects.find(name) != soundEffects.end()) {
            return true; // Already loaded
        }

        Mix_Chunk *sound = Mix_LoadWAV(path.c_str());
        if (!sound) {
            std::cerr << "Failed to load sound effect: " << Mix_GetError() << std::endl;
            return false;
        }

        soundEffects[name] = sound;
        return true;
    }

    // Load background music
    bool loadMusic(const std::string &name, const std::string &path) {
        if (musicTracks.find(name) != musicTracks.end()) {
            return true; // Already loaded
        }

        Mix_Music *music = Mix_LoadMUS(path.c_str());
        if (!music) {
            std::cerr << "Failed to load music: " << Mix_GetError() << std::endl;
            return false;
        }

        musicTracks[name] = music;
        return true;
    }

    // Play a sound effect
    void playSound(const std::string &name, float volume = 1.0f, int loops = 0) {
        auto it = soundEffects.find(name);
        if (it == soundEffects.end()) {
            std::cerr << "Sound effect not found: " << name << std::endl;
            return;
        }

        int channel = Mix_PlayChannel(-1, it->second, loops);
        if (channel == -1) {
            std::cerr << "Failed to play sound effect: " << Mix_GetError() << std::endl;
            return;
        }

        Mix_Volume(channel, static_cast<int>(MIX_MAX_VOLUME * volume * masterVolume * sfxVolume));
    }

    // Play background music
    void playMusic(const std::string &name, float volume = 1.0f, bool loop = true) {
        auto it = musicTracks.find(name);
        if (it == musicTracks.end()) {
            std::cerr << "Music track not found: " << name << std::endl;
            return;
        }

        if (Mix_PlayMusic(it->second, loop ? -1 : 0) == -1) {
            std::cerr << "Failed to play music: " << Mix_GetError() << std::endl;
            return;
        }

        Mix_VolumeMusic(static_cast<int>(MIX_MAX_VOLUME * volume * masterVolume * musicVolume));
    }

    // Stop all sounds
    void stopAllSounds() {
        Mix_HaltChannel(-1);
    }

    // Stop music
    void stopMusic() {
        Mix_HaltMusic();
    }

    // Pause music
    void pauseMusic() {
        Mix_PauseMusic();
    }

    // Resume music
    void resumeMusic() {
        Mix_ResumeMusic();
    }

    // Set volume levels
    void setMasterVolume(float volume) {
        masterVolume = std::clamp(volume, 0.0f, 1.0f);
        updateVolumes();
    }

    void setMusicVolume(float volume) {
        musicVolume = std::clamp(volume, 0.0f, 1.0f);
        updateVolumes();
    }

    void setSFXVolume(float volume) {
        sfxVolume = std::clamp(volume, 0.0f, 1.0f);
        updateVolumes();
    }

private:
    void updateVolumes() {
        // Update music volume
        Mix_VolumeMusic(static_cast<int>(MIX_MAX_VOLUME * masterVolume * musicVolume));

        // Update all sound effect channels
        for (int i = 0; i < CHANNELS; i++) {
            if (Mix_Playing(i)) {
                Mix_Volume(i, static_cast<int>(MIX_MAX_VOLUME * masterVolume * sfxVolume));
            }
        }
    }
};

#endif //SOUND_HPP
