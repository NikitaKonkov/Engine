#include <audio/audio.hpp>
#include <SDL3/SDL.h>
#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioSystem::AudioSystem() : audioDeviceID(0), audioStream(nullptr), sampleRate(48000), frequency(440.0f), isPlaying(false) {
    GenerateSineWave();
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
    // 1 second of audio
    sineWaveData.resize(sampleRate);
    for (int i = 0; i < sampleRate; i++) {
        double time = static_cast<double>(i) / sampleRate;
        sineWaveData[i] = static_cast<float>(sin(2.0 * M_PI * frequency * time)) * 0.2f; // 0.2f for volume control
    }
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
    
    std::cout << "Audio system initialized successfully!" << std::endl;
    return true;
}

void AudioSystem::PlaySound() {
    if (audioDeviceID > 0 && audioStream) {
        // Put the sine wave data into the stream
        if (!SDL_PutAudioStreamData(audioStream, sineWaveData.data(), 
                                   static_cast<int>(sineWaveData.size() * sizeof(float)))) {
            std::cerr << "Failed to put audio data: " << SDL_GetError() << std::endl;
            return;
        }
        
        SDL_ResumeAudioDevice(audioDeviceID);
        std::cout << "Playing sound..." << std::endl;
    }
}

void AudioSystem::StopSound() {
    if (audioDeviceID > 0) {
        SDL_PauseAudioDevice(audioDeviceID);
        std::cout << "Sound stopped." << std::endl;
    }
}

void AudioSystem::SetFrequency(float newFreq) {
    frequency = newFreq;
    GenerateSineWave();
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
    
    // Stop any previous thread that might still be running
    StopAsyncSound();
    
    // Start a new thread to play the sound
    isPlaying.store(true);
    audioThread = std::thread([this, durationMs]() {
        this->PlaySound();
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

// Example function to play a simple sound asynchronously
void PlaySimpleSoundAsync(int durationMs = 1000) {
    static AudioSystem* audioSystem = nullptr;
    static std::mutex audioMutex;
    
    std::lock_guard<std::mutex> lock(audioMutex);
    
    // Create the audio system if it doesn't exist
    if (!audioSystem) {
        audioSystem = new AudioSystem();
        if (!audioSystem->Initialize()) {
            delete audioSystem;
            audioSystem = nullptr;
            std::cerr << "Failed to initialize audio system" << std::endl;
            return;
        }
    }
    
    // Only play if no sound is currently playing
    if (!audioSystem->IsPlaying()) {
        audioSystem->SetFrequency(440.0f);
        audioSystem->PlaySoundAsync(durationMs);
    } else {
        std::cout << "Sound is already playing. Wait for it to finish." << std::endl;
    }
}

// Original synchronous version
void PlaySimpleSound() {
    AudioSystem audioSystem;
    
    if (audioSystem.Initialize()) {
        std::cout << "Playing a simple sound..." << std::endl;
        
        // Play a basic sound at 440Hz (A4 note)
        audioSystem.SetFrequency(440.0f);
        audioSystem.PlaySound();
        SDL_Delay(1000);
        // Play for 2 seconds
        audioSystem.StopSound();
    }
}
