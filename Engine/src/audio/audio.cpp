#include <audio/audio.hpp>
#include <SDL3/SDL.h>
#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <map>
#include <inputs/keyboard.hpp>  // Add this include
#include <audio/mixer.hpp>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioSystem::AudioSystem() : audioDeviceID(0), audioStream(nullptr), sampleRate(48000), frequency(440.0f), isPlaying(false), fadeSamples(1000) {
    // Add default sine wave component
    AddWaveComponent(WaveType::Sine, frequency, 0.2f);
    GenerateComplexWave();
}

AudioSystem::~AudioSystem() {
    StopAsyncSound();
    if (audioStream) {
        SDL_DestroyAudioStream(audioStream);
    }
    if (audioDeviceID > 0) {
        SDL_CloseAudioDevice(audioDeviceID);
    }
}

// Generate a simple sine wave
void AudioSystem::GenerateSineWave() {
    // 1 second of audio with additional space for fade in/out
    sineWaveData.resize(sampleRate);
    
    // Initialize with silence
    std::fill(sineWaveData.begin(), sineWaveData.end(), 0.0f);
    
    // Generate the sine wave
    for (int i = 0; i < sampleRate; i++) {
        double time = static_cast<double>(i) / sampleRate;
        sineWaveData[i] = static_cast<float>(sin(2.0 * M_PI * frequency * time)) * 0.8f; // 0.8f for volume control
    }
    
    // Apply fades
    ApplyFades();
}

float AudioSystem::GenerateWaveSample(WaveType type, float phase, float amplitude) {
    switch (type) {
        case WaveType::Sine:
            return sin(phase) * amplitude;
        
        case WaveType::Square:
            return (sin(phase) > 0 ? 1.0f : -1.0f) * amplitude;
            
        case WaveType::Triangle:
            return (2.0f / M_PI) * asin(sin(phase)) * amplitude;
            
        case WaveType::Sawtooth:
            return (2.0f / M_PI) * atan(tan(phase / 2.0f)) * amplitude;
            
        default:
            return 0.0f;
    }
}

void AudioSystem::AddWaveComponent(WaveType type, float freq, float amplitude) {
    WaveComponent component{type, freq, amplitude};
    waveComponents.push_back(component);
}

void AudioSystem::ClearWaveComponents() {
    waveComponents.clear();
}

void AudioSystem::ApplyFades() {
    // Apply fade-in to the first part of the waveform
    for (int i = 0; i < fadeSamples && i < sineWaveData.size(); i++) {
        float fadeProgress = static_cast<float>(i) / fadeSamples;
        // Apply a smooth sine fade-in
        float fadeMultiplier = sin(fadeProgress * (M_PI / 2.0f));
        sineWaveData[i] *= fadeMultiplier;
    }
    
    // Apply fade-out to the last part of the waveform
    size_t fadeOutStart = sineWaveData.size() - fadeSamples;
    for (size_t i = fadeOutStart; i < sineWaveData.size(); i++) {
        float fadeProgress = static_cast<float>(sineWaveData.size() - i) / fadeSamples;
        // Apply a smooth sine fade-out
        float fadeMultiplier = sin(fadeProgress * (M_PI / 2.0f));
        sineWaveData[i] *= fadeMultiplier;
    }
}

void AudioSystem::GenerateComplexWave() {
    // If no components are defined, use the basic sine wave
    if (waveComponents.empty()) {
        GenerateSineWave();
        return;
    }
    
    // Generate combined waveform
    sineWaveData.resize(sampleRate);
    
    // Initialize buffer with silence
    std::fill(sineWaveData.begin(), sineWaveData.end(), 0.0f);
    
    for (int i = 0; i < sampleRate; i++) {
        double time = static_cast<double>(i) / sampleRate;
        float sample = 0.0f;
        
        // Sum all wave components
        for (const auto& comp : waveComponents) {
            float phase = 2.0f * M_PI * comp.frequency * time;
            sample += GenerateWaveSample(comp.type, phase, comp.amplitude);
        }
        
        // Normalize to prevent clipping
        if (!waveComponents.empty()) {
            float normalizer = 1.0f / waveComponents.size();
            sample *= normalizer; 
        }
        
        sineWaveData[i] = sample;
    }
    
    // Apply fades to smooth out the beginning and end
    ApplyFades();
}

bool AudioSystem::Initialize() {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        std::cerr << "Failed to initialize SDL audio: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Setup the audio specification
    SDL_zero(audioSpec);
    audioSpec.freq = sampleRate;
    audioSpec.format = SDL_AUDIO_F32;
    audioSpec.channels = 1;
    
    // Open audio device with the default playback device
    audioDeviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec);
    if (audioDeviceID == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create audio stream
    audioStream = SDL_CreateAudioStream(&audioSpec, &audioSpec);
    if (!audioStream) {
        std::cerr << "Failed to create audio stream: " << SDL_GetError() << std::endl;
        SDL_CloseAudioDevice(audioDeviceID);
        return false;
    }
    
    // Bind the stream to the device
    if (!SDL_BindAudioStream(audioDeviceID, audioStream)) {
        std::cerr << "Failed to bind audio stream: " << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(audioStream);
        SDL_CloseAudioDevice(audioDeviceID);
        return false;
    }
    
    // Before playing any sound, clear the audio buffer
    SDL_ClearAudioStream(audioStream);
    
    std::cout << "Audio system initialized successfully!" << std::endl;
    return true;
}

void AudioSystem::PlaySound() {
    if (audioDeviceID > 0 && audioStream) {
        // Clear any previous data in the stream
        SDL_ClearAudioStream(audioStream);
        
        // Add 50ms of silence before the actual sound to ensure clean start
        std::vector<float> silenceBuffer(sampleRate / 20, 0.0f);
        SDL_PutAudioStreamData(audioStream, silenceBuffer.data(), static_cast<int>(silenceBuffer.size() * sizeof(float)));
        
        // Put the sine wave data into the stream
        if (!SDL_PutAudioStreamData(audioStream, sineWaveData.data(), 
                                   static_cast<int>(sineWaveData.size() * sizeof(float)))) {
            std::cerr << "Failed to put audio data: " << SDL_GetError() << std::endl;
            return;
        }
        
        // Add 50ms of silence after the actual sound to ensure clean end
        SDL_PutAudioStreamData(audioStream, silenceBuffer.data(), static_cast<int>(silenceBuffer.size() * sizeof(float)));
        
        SDL_ResumeAudioDevice(audioDeviceID);
        std::cout << "Playing sound..." << std::endl;
    }
}

void AudioSystem::StopSound() {
    if (audioDeviceID > 0) {
        // Instead of an abrupt stop, fade out gracefully
        // First clear the stream
        SDL_ClearAudioStream(audioStream);
        
        // Then pause the device
        SDL_PauseAudioDevice(audioDeviceID);
        std::cout << "Sound stopped." << std::endl;
    }
}

void AudioSystem::SetFrequency(float newFreq) {
    frequency = newFreq;
    
    // Update the main frequency in the first wave component or add one if empty
    if (!waveComponents.empty()) {
        waveComponents[0].frequency = frequency;
    } else {
        AddWaveComponent(WaveType::Sine, frequency, 1.0f);
    }
    
    GenerateComplexWave();
    std::cout << "Frequency set to " << frequency << " Hz" << std::endl;
}

bool AudioSystem::IsPlaying() const {
    return isPlaying;
}

void AudioSystem::PlaySoundAsync(int durationMs) {
    // If a sound is already playing, don't start another one
    if (isPlaying.load()) {
        std::cout << "Sound is already playing. Wait for it to finish." << std::endl;
        return;
    }
    
    // Start a new thread to play the sound
    isPlaying.store(true);
    audioThread = std::thread([this, durationMs]() {
        this->PlaySound();
        
        // Sleep for the specified duration
        SDL_Delay(durationMs);
        
        this->StopSound();
        this->isPlaying.store(false);
        std::cout << "Async sound playback finished." << std::endl;
    });
    
    // Detach the thread to let it run independently
    audioThread.detach();
    std::cout << "Started async sound playback for " << durationMs << " ms" << std::endl;
}

void AudioSystem::StopAsyncSound() {
    if (isPlaying.load()) {
        StopSound();
        isPlaying.store(false);
        std::cout << "Async sound playback stopped." << std::endl;
    }
}

// Fixed synchronous version with multiple wave components
void PlaySimpleSound() {
    // Use the mixer to play a simple sound instead
    if (gAudioMixer == nullptr) {
        InitializeAudioMixer();
    }
    
    // Create a sample called "simpleSound"
    gAudioMixer->AddSample("simpleSound", WaveType::Sine, 82.0f, 0.3f);
    gAudioMixer->AddSample("simpleSound", WaveType::Sine, 164.0f, 0.15f);
    gAudioMixer->AddSample("simpleSound", WaveType::Triangle, 123.0f, 0.8f);
    
    // Play the sample
    gAudioMixer->PlaySample("simpleSound", 1000);
}

// Global piano functionality functions that are referenced in keyboard.hpp
void PlaySimpleSoundAsync(int durationMs, float frequency) {
    // Use the mixer to play a simple sound with the given frequency
    if (gAudioMixer == nullptr) {
        InitializeAudioMixer();
    }
    
    // Play the sound with the given frequency and duration
    gAudioMixer->PlaySound(frequency, durationMs);
    std::cout << "Playing simple sound async: " << frequency << "Hz for " << durationMs << "ms" << std::endl;
}

void ToggleSustainMode() {
    // Use the mixer to toggle sustain mode
    if (gAudioMixer == nullptr) {
        InitializeAudioMixer();
    }
    
    gAudioMixer->ToggleSustainMode();
}

void StopAllSounds() {
    // Use the mixer to stop all sounds
    if (gAudioMixer == nullptr) {
        InitializeAudioMixer();
    }
    
    gAudioMixer->StopAllSounds();
    std::cout << "All sounds stopped" << std::endl;
}
