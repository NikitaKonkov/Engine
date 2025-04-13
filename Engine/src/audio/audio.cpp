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

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Global variables for multi-channel audio management
std::map<int, AudioSystem*> audioChannels;
std::mutex channelsMutex;
bool longSustainMode = false; // For piano sustain mode

AudioSystem::AudioSystem() : audioDeviceID(0), audioStream(nullptr), sampleRate(48000), frequency(440.0f), isPlaying(false) {
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
    // 1 second of audio
    sineWaveData.resize(sampleRate);
    for (int i = 0; i < sampleRate; i++) {
        double time = static_cast<double>(i) / sampleRate;
        sineWaveData[i] = static_cast<float>(sin(2.0 * M_PI * frequency * time)) * 0.2f; // 0.2f for volume control
    }
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

void AudioSystem::GenerateComplexWave() {
    // If no components are defined, use the basic sine wave
    if (waveComponents.empty()) {
        GenerateSineWave();
        return;
    }
    
    // Generate combined waveform
    sineWaveData.resize(sampleRate);
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

// Example function to play a simple sound asynchronously with channel support
void PlaySimpleSoundAsync(int durationMs, float frequency) {
    static int nextChannelId = 1;
    std::lock_guard<std::mutex> lock(channelsMutex);
    
    int actualDuration = longSustainMode ? 200 : 2000; // Use longer duration if sustain mode is on
    
    // Create a new audio system for this sound
    int channelId = nextChannelId++;
    AudioSystem* audioSystem = new AudioSystem();
    
    if (!audioSystem->Initialize()) {
        delete audioSystem;
        std::cerr << "Failed to initialize audio system for channel " << channelId << std::endl;
        return;
    }
    
    // Store the channel
    audioChannels[channelId] = audioSystem;
    
    // Configure and play the sound
    audioSystem->SetFrequency(frequency);
    
    // Start a new thread to play the sound and clean up when done
    std::thread([channelId, audioSystem, actualDuration]() {
        audioSystem->PlaySound();
        SDL_Delay(actualDuration);
        audioSystem->StopSound();
        
        // Clean up after sound finishes
        std::lock_guard<std::mutex> cleanupLock(channelsMutex);
        if (audioChannels.find(channelId) != audioChannels.end()) {
            delete audioSystem;
            audioChannels.erase(channelId);
        }
    }).detach();
}

// Function to toggle sustain mode
void ToggleSustainMode() {
    longSustainMode = !longSustainMode;
    std::cout << "Piano sustain mode: " << (longSustainMode ? "ON" : "OFF") << std::endl;
}

// Function to stop all playing sounds
void StopAllSounds() {
    std::lock_guard<std::mutex> lock(channelsMutex);
    for (auto& [channelId, system] : audioChannels) {
        system->StopSound();
    }
}

// Fixed synchronous version with multiple wave components
void PlaySimpleSound() {
    AudioSystem audioSystem;
    
    if (audioSystem.Initialize()) {
        SDL_Log("Playing a complex sound...");
        
        // Clear any existing waves
        audioSystem.ClearWaveComponents();
        
        // Add multiple wave components to create a richer sound
        audioSystem.AddWaveComponent(WaveType::Sine, 82.0f, 0.3f);        // Base frequency
        audioSystem.AddWaveComponent(WaveType::Sine, 164.0f, 0.15f);      // First harmonic (2x)
        audioSystem.AddWaveComponent(WaveType::Triangle, 123.0f, 0.8f);  // Add some complexity
        
        // Generate the complex wave from the components
        audioSystem.GenerateComplexWave();
        
        // Play the sound
        audioSystem.PlaySound();
        SDL_Delay(1000);
        
        audioSystem.StopSound();
    }
}
