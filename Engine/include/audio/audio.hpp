#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <thread>
#include <atomic>

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();
    
    bool Initialize();
    void PlaySound();
    void StopSound();
    void SetFrequency(float freq);
    
    // New asynchronous methods
    void PlaySoundAsync(int durationMs);
    void StopAsyncSound();
    bool IsPlaying() const;
    
private:
    void GenerateSineWave();
    
    SDL_AudioDeviceID audioDeviceID;
    SDL_AudioStream* audioStream;
    SDL_AudioSpec audioSpec;
    std::vector<float> sineWaveData;
    int sampleRate;
    float frequency;
    
    // For async playback
    std::atomic<bool> isPlaying;
    std::thread audioThread;
};

// Helper functions for simpler usage
void PlaySimpleSound();
void PlaySimpleSoundAsync(int durationMs);