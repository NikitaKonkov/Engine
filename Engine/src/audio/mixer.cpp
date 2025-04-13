#include <audio/mixer.hpp>
#include <audio/audio.hpp>
#include <iostream>
#include <SDL3/SDL.h>

// Global mixer instance
AudioMixer* gAudioMixer = nullptr;

AudioMixer::AudioMixer() : nextChannelId(1), longSustainMode(false) {
}

AudioMixer::~AudioMixer() {
    StopAllSounds();
}

bool AudioMixer::Initialize() {
    std::cout << "Audio mixer initialized" << std::endl;
    return true;
}

int AudioMixer::PlaySound(float frequency, int durationMs) {
    std::lock_guard<std::mutex> lock(channelsMutex);
    
    int actualDuration = longSustainMode ? 5000 : durationMs; // Use longer duration if sustain mode is on
    
    // Create a new audio system for this sound
    int channelId = nextChannelId++;
    auto audioSystem = std::make_unique<AudioSystem>();
    
    if (!audioSystem->Initialize()) {
        std::cerr << "Failed to initialize audio system for channel " << channelId << std::endl;
        return -1;
    }
    
    // Configure and play the sound
    audioSystem->SetFrequency(frequency);
    
    // Start a new thread to play the sound and clean up when done
    std::thread([this, channelId, audioSystem = audioSystem.get(), actualDuration]() {
        audioSystem->PlaySound();
        SDL_Delay(actualDuration);
        audioSystem->StopSound();
        
        // Clean up after sound finishes
        std::lock_guard<std::mutex> cleanupLock(this->channelsMutex);
        this->audioChannels.erase(channelId);
    }).detach();
    
    // Store the channel (move ownership to the map)
    audioChannels[channelId] = std::move(audioSystem);
    
    return channelId;
}

void AudioMixer::StopSound(int channelId) {
    std::lock_guard<std::mutex> lock(channelsMutex);
    auto it = audioChannels.find(channelId);
    if (it != audioChannels.end()) {
        it->second->StopSound();
        audioChannels.erase(it);
    }
}

void AudioMixer::StopAllSounds() {
    std::lock_guard<std::mutex> lock(channelsMutex);
    for (auto& pair : audioChannels) {
        pair.second->StopSound();
    }
    audioChannels.clear();
}

void AudioMixer::AddSample(const std::string& name, WaveType type, float freq, float amplitude) {
    WaveComponent component{type, freq, amplitude};
    
    // If sample doesn't exist, create it
    if (samples.find(name) == samples.end()) {
        samples[name] = std::vector<WaveComponent>();
    }
    
    samples[name].push_back(component);
    std::cout << "Added " << static_cast<int>(type) << " wave component to sample '" << name << "'" << std::endl;
}

void AudioMixer::PlaySample(const std::string& name, int durationMs) {
    auto it = samples.find(name);
    if (it == samples.end()) {
        std::cerr << "Sample '" << name << "' not found" << std::endl;
        return;
    }
    
    std::lock_guard<std::mutex> lock(channelsMutex);
    
    int actualDuration = longSustainMode ? 5000 : durationMs;
    
    // Create a new audio system for this sample
    int channelId = nextChannelId++;
    auto audioSystem = std::make_unique<AudioSystem>();
    
    if (!audioSystem->Initialize()) {
        std::cerr << "Failed to initialize audio system for sample '" << name << "'" << std::endl;
        return;
    }
    
    // Clear default wave components
    audioSystem->ClearWaveComponents();
    
    // Add all wave components from the sample
    for (const auto& component : it->second) {
        audioSystem->AddWaveComponent(component.type, component.frequency, component.amplitude);
    }
    
    // Generate the complex wave
    audioSystem->GenerateComplexWave();
    
    // Start a new thread to play the sound and clean up when done
    std::thread([this, channelId, audioSystem = audioSystem.get(), actualDuration, name]() {
        audioSystem->PlaySound();
        SDL_Delay(actualDuration);
        audioSystem->StopSound();
        
        // Clean up after sound finishes
        std::lock_guard<std::mutex> cleanupLock(this->channelsMutex);
        this->audioChannels.erase(channelId);
        std::cout << "Sample '" << name << "' playback finished" << std::endl;
    }).detach();
    
    // Store the channel (move ownership to the map)
    audioChannels[channelId] = std::move(audioSystem);
    std::cout << "Playing sample '" << name << "' for " << actualDuration << "ms" << std::endl;
}

void AudioMixer::ClearSamples() {
    samples.clear();
    std::cout << "All samples cleared" << std::endl;
}

void AudioMixer::ToggleSustainMode() {
    longSustainMode = !longSustainMode;
    std::cout << "Sustain mode: " << (longSustainMode ? "ON" : "OFF") << std::endl;
}

bool AudioMixer::IsSustainModeEnabled() const {
    return longSustainMode;
}

// Global helper functions
void InitializeAudioMixer() {
    if (!gAudioMixer) {
        gAudioMixer = new AudioMixer();
        gAudioMixer->Initialize();
    }
}

void ShutdownAudioMixer() {
    if (gAudioMixer) {
        delete gAudioMixer;
        gAudioMixer = nullptr;
    }
}