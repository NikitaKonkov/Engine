#pragma once

#include <SDL3/SDL_audio.h>
#include <vector>

class AudioSystem {
private:
    SDL_AudioDeviceID audioDeviceID;
    SDL_AudioSpec audioSpec;
    SDL_AudioStream* audioStream;
    std::vector<float> sineWaveData;
    int sampleRate;
    float frequency;
    
public:
    AudioSystem();
    ~AudioSystem();
    
    void GenerateSineWave();
    bool Initialize();
    void PlaySound();
    void StopSound();
    void SetFrequency(float newFreq);
};

// Example function
void PlaySimpleSound();