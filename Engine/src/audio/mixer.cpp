#include <audio/mixer.hpp>
#include <audio/audio.hpp>
#include <iostream>
#include <SDL3/SDL.h>
#include <cmath>

// Global mixer instance
AudioMixer* gAudioMixer = nullptr;

AudioMixer::AudioMixer() : nextChannelId(1), longSustainMode(false), fadeOutDuration(15) {
    // Default fade-out duration is 15ms
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
    auto channel = std::make_unique<ActiveChannel>();
    channel->audioSystem = std::make_unique<AudioSystem>();
    channel->isActive = true;
    channel->isFadingOut = false;
    channel->startTime = SDL_GetTicks();
    channel->fadeDuration = fadeOutDuration;
    channel->totalDuration = actualDuration;
    
    if (!channel->audioSystem->Initialize()) {
        std::cerr << "Failed to initialize audio system for channel " << channelId << std::endl;
        return -1;
    }
    
    // Configure and play the sound
    channel->audioSystem->SetFrequency(frequency);
    
    // Start playback
    channel->audioSystem->PlaySound();
    
    // Start a new thread to manage the sound's lifecycle
    std::thread([this, channelId, actualDuration]() {
        // Wait until just before the end of the sound duration
        int timeToSleep = actualDuration - static_cast<int>(this->fadeOutDuration) - 5; // 5ms safety margin
        if (timeToSleep > 0) {
            SDL_Delay(timeToSleep);
        }
        
        // Initiate fade-out
        {
            std::lock_guard<std::mutex> fadelock(this->channelsMutex);
            auto it = this->audioChannels.find(channelId);
            if (it != this->audioChannels.end() && it->second->isActive) {
                this->StartFadeOut(it->second.get());
            }
        }
        
        // Wait for fade-out to complete
        SDL_Delay(this->fadeOutDuration + 10);
        
        // Clean up after sound finishes
        std::lock_guard<std::mutex> cleanupLock(this->channelsMutex);
        auto it = this->audioChannels.find(channelId);
        if (it != this->audioChannels.end()) {
            it->second->audioSystem->StopSound();
            this->audioChannels.erase(it);
        }
    }).detach();
    
    // Store the channel (move ownership to the map)
    audioChannels[channelId] = std::move(channel);
    
    return channelId;
}

void AudioMixer::StopSound(int channelId) {
    std::lock_guard<std::mutex> lock(channelsMutex);
    auto it = audioChannels.find(channelId);
    if (it != audioChannels.end()) {
        // Start fade-out instead of stopping abruptly
        StartFadeOut(it->second.get());
        
        // The channel will be removed after fade-out completes in the Update method
    }
}

void AudioMixer::StopAllSounds() {
    std::lock_guard<std::mutex> lock(channelsMutex);
    for (auto& pair : audioChannels) {
        // Initiate fade-out for each active channel
        StartFadeOut(pair.second.get());
    }
    
    // Wait a small amount of time for fades to process
    SDL_Delay(fadeOutDuration + 10);
    
    // Now stop all sounds and clear the channels
    for (auto& pair : audioChannels) {
        pair.second->audioSystem->StopSound();
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
    auto channel = std::make_unique<ActiveChannel>();
    channel->audioSystem = std::make_unique<AudioSystem>();
    channel->isActive = true;
    channel->isFadingOut = false;
    channel->startTime = SDL_GetTicks();
    channel->fadeDuration = fadeOutDuration;
    channel->totalDuration = actualDuration;
    
    if (!channel->audioSystem->Initialize()) {
        std::cerr << "Failed to initialize audio system for sample '" << name << "'" << std::endl;
        return;
    }
    
    // Clear default wave components
    channel->audioSystem->ClearWaveComponents();
    
    // Add all wave components from the sample
    for (const auto& component : it->second) {
        channel->audioSystem->AddWaveComponent(component.type, component.frequency, component.amplitude);
    }
    
    // Generate the complex wave
    channel->audioSystem->GenerateComplexWave();
    
    // Start playback
    channel->audioSystem->PlaySound();
    
    // Start a new thread to manage the sound's lifecycle
    std::thread([this, channelId, name, actualDuration]() {
        // Wait until just before the end of the sound duration
        int timeToSleep = actualDuration - static_cast<int>(this->fadeOutDuration) - 50; // 5ms safety margin
        if (timeToSleep > 0) {
            SDL_Delay(timeToSleep);
        }
        
        // Initiate fade-out
        {
            std::lock_guard<std::mutex> fadelock(this->channelsMutex);
            auto it = this->audioChannels.find(channelId);
            if (it != this->audioChannels.end() && it->second->isActive) {
                this->StartFadeOut(it->second.get());
            }
        }
        
        // Wait for fade-out to complete
        SDL_Delay(this->fadeOutDuration + 10);
        
        // Clean up after sound finishes
        std::lock_guard<std::mutex> cleanupLock(this->channelsMutex);
        auto it = this->audioChannels.find(channelId);
        if (it != this->audioChannels.end()) {
            it->second->audioSystem->StopSound();
            this->audioChannels.erase(it);
            std::cout << "Sample '" << name << "' playback finished" << std::endl;
        }
    }).detach();
    
    // Store the channel (move ownership to the map)
    audioChannels[channelId] = std::move(channel);
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

void AudioMixer::SetFadeOutDuration(uint64_t durationMs) {
    fadeOutDuration = durationMs;
    std::cout << "Fade-out duration set to " << fadeOutDuration << "ms" << std::endl;
}

uint64_t AudioMixer::GetFadeOutDuration() const {
    return fadeOutDuration;
}

void AudioMixer::StartFadeOut(ActiveChannel* channel) {
    if (!channel->isFadingOut) {
        channel->isFadingOut = true;
        // Here would be a good place for debug info
        // std::cout << "Starting fade-out over " << channel->fadeDuration << "ms" << std::endl;
    }
}

float AudioMixer::CalculateFadeOutVolume(uint64_t currentTime, uint64_t startFadeTime, uint64_t fadeDuration) {
    // Calculate elapsed time since fade started
    uint64_t elapsed = currentTime - startFadeTime;
    
    // If we're beyond the fade duration, return 0 (silent)
    if (elapsed >= fadeDuration) {
        return 0.0f;
    }
    
    // Calculate linear fade-out volume (1.0 -> 0.0)
    float fadeRatio = static_cast<float>(elapsed) / static_cast<float>(fadeDuration);
    
    // Apply a smoother curve (cubic or cosine curve sounds more natural)
    // Cosine interpolation: 0.5 * (1 + cos(π * fadeRatio))
    return 0.5f * (1.0f + cosf(fadeRatio * 3.14159f));
}

void AudioMixer::Update() {
    std::lock_guard<std::mutex> lock(channelsMutex);
    uint64_t currentTime = SDL_GetTicks();
    
    // Process all active channels
    for (auto it = audioChannels.begin(); it != audioChannels.end(); ) {
        ActiveChannel* channel = it->second.get();
        
        // Check if fade-out is in progress
        if (channel->isFadingOut) {
            // Calculate how much time has passed since the fade-out started
            uint64_t fadeStartTime = channel->startTime + channel->totalDuration - channel->fadeDuration;
            
            // Calculate the volume based on fade-out progress
            float volume = CalculateFadeOutVolume(currentTime, fadeStartTime, channel->fadeDuration);
            
            // Apply the volume change to the audio system
            // Note: This would require adding volume control to AudioSystem class
            // For now, we'll just stop the sound once fade reaches near zero
            if (volume <= 0.001f) {
                // Fade complete, stop the sound
                it->second->audioSystem->StopSound();
                it = audioChannels.erase(it);
                continue;
            }
        }
        
        // Move to next channel
        ++it;
    }
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