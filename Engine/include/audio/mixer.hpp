#pragma once

#include <audio/audio.hpp>
#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <atomic>

// Structure for active channel information
struct ActiveChannel {
    std::unique_ptr<AudioSystem> audioSystem;
    std::atomic<bool> isActive;
    std::atomic<bool> isFadingOut;
    uint64_t startTime;
    uint64_t fadeDuration; // in milliseconds
    int totalDuration;     // in milliseconds
};

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
    
    // Fade settings
    void SetFadeOutDuration(uint64_t durationMs);
    uint64_t GetFadeOutDuration() const;
    
    // Update method to handle fade-outs
    void Update();

private:
    std::map<int, std::unique_ptr<ActiveChannel>> audioChannels;
    std::mutex channelsMutex;
    int nextChannelId;
    bool longSustainMode;
    uint64_t fadeOutDuration; // in milliseconds
    
    // Helper method for initiating fade-out
    void StartFadeOut(ActiveChannel* channel);
    
    // Helper method for applying fade-out volume scaling
    float CalculateFadeOutVolume(uint64_t currentTime, uint64_t startFadeTime, uint64_t fadeDuration);

    // Sample storage
    std::map<std::string, std::vector<WaveComponent>> samples;
};

// Global mixer instance
extern AudioMixer* gAudioMixer;

// Helper functions
void InitializeAudioMixer();
void ShutdownAudioMixer();