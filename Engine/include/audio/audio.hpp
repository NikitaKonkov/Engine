#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <thread>
#include <atomic>

// Define wave types
enum class WaveType {
    Sine,
    Square,
    Triangle,
    Sawtooth
};

// Structure to define a wave component
struct WaveComponent {
    WaveType type;
    float frequency;
    float amplitude;
};

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();
    
    bool Initialize();
    void PlaySound();
    void StopSound();
    void SetFrequency(float freq);
    
    // New methods for handling multiple waves
    void AddWaveComponent(WaveType type, float freq, float amplitude);
    void ClearWaveComponents();
    void GenerateComplexWave();
    
    // New asynchronous methods
    void PlaySoundAsync(int durationMs);
    void StopAsyncSound();
    bool IsPlaying() const;
    
private:
    void GenerateSineWave();
    float GenerateWaveSample(WaveType type, float phase, float amplitude);
    void ApplyFades(); // New method to apply fade-in and fade-out effects
    
    SDL_AudioDeviceID audioDeviceID;
    SDL_AudioStream* audioStream;
    SDL_AudioSpec audioSpec;
    std::vector<float> sineWaveData;
    int sampleRate;
    float frequency;
    
    // For complex waveforms
    std::vector<WaveComponent> waveComponents;
    
    // For async playback
    std::atomic<bool> isPlaying;
    std::thread audioThread;
    
    // For fade effects
    int fadeSamples; // Number of samples for fades
};

// Helper functions for simpler usage
void PlaySimpleSound();

// Piano functionality
void PlaySimpleSoundAsync(int durationMs, float frequency);
void ToggleSustainMode();
void StopAllSounds();