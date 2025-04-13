#pragma once

#include <audio/audio.hpp>
#include <map>
#include <mutex>
#include <memory>
#include <string>

class AudioMixer {
public:
    AudioMixer();
    ~AudioMixer();

    // Initialize the audio mixer
    bool Initialize();

    // Channel management
    int PlaySound(float frequency, int durationMs);
    void StopSound(int channelId);
    void StopAllSounds();

    // Sample management
    void AddSample(const std::string& name, WaveType type, float freq, float amplitude);
    void PlaySample(const std::string& name, int durationMs);
    void ClearSamples();

    // Audio mode controls
    void ToggleSustainMode();
    bool IsSustainModeEnabled() const;

private:
    std::map<int, std::unique_ptr<AudioSystem>> audioChannels;
    std::mutex channelsMutex;
    int nextChannelId;
    bool longSustainMode;

    // Sample storage
    std::map<std::string, std::vector<WaveComponent>> samples;
};

// Global mixer instance
extern AudioMixer* gAudioMixer;

// Helper functions
void InitializeAudioMixer();
void ShutdownAudioMixer();