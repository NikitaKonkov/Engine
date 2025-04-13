#include <assets/piano/piano.hpp>
#include <audio/mixer.hpp>
#include <audio/audio.hpp>
#include <iostream>

// Global piano instance
Piano* gPiano = nullptr;

Piano::Piano() : sustainMode(false) {
    initializeNoteFrequencies();
    initializeKeyMappings();
}

Piano::~Piano() {
    stopAllNotes();
}

bool Piano::initialize() {
    // Make sure audio mixer is initialized
    if (gAudioMixer == nullptr) {
        InitializeAudioMixer();
    }
    
    std::cout << "Piano system initialized" << std::endl;
    return true;
}

void Piano::update() {
    SDL_Delay(100); // Simulate a small delay for the update loop
    // Handle any updates required for the piano system
}

void Piano::handleKeyEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event.key.key;
        // Check if this key is mapped to a piano note
        auto noteIt = keyToNoteMap.find(key);
        if (noteIt != keyToNoteMap.end()) {
            playNote(noteIt->second);
        }
        

    }
}

void Piano::playNote(const std::string& note, int durationMs) {
    auto freqIt = noteFrequencies.find(note);
    if (freqIt != noteFrequencies.end()) {
        float frequency = freqIt->second;
        
        // Use the audio mixer to play the note
        if (gAudioMixer) {
            gAudioMixer->PlaySound(frequency, durationMs);
            std::cout << "Playing note " << note << " at " << frequency << "Hz" << std::endl;
        }
    }
}

void Piano::toggleSustainMode() {
    sustainMode = !sustainMode;
    
    // Update the audio mixer's sustain mode
    if (gAudioMixer) {
        gAudioMixer->ToggleSustainMode();
    }
    
    std::cout << "Sustain mode: " << (sustainMode ? "ON" : "OFF") << std::endl;
}

void Piano::stopAllNotes() {
    if (gAudioMixer) {
        gAudioMixer->StopAllSounds();
    }
}

bool Piano::isSustainModeEnabled() const {
    return sustainMode;
}

void Piano::initializeNoteFrequencies() {
    // Initialize musical note frequencies (using standard A4 = 440Hz as reference)
    noteFrequencies = {
        {"C4", 261.63f},  // C4
        {"D4", 293.66f},  // D4
        {"E4", 329.63f},  // E4
        {"F4", 349.23f},  // F4
        {"G4", 392.00f},  // G4
        {"A4", 440.00f},  // A4
        {"B4", 493.88f},  // B4
        {"C5", 523.25f},  // C5
        {"D5", 587.33f},  // D5
        {"E5", 659.25f}   // E5
    };
}

void Piano::initializeKeyMappings() {
    // Map keyboard keys to notes
    keyToNoteMap = {
        {SDLK_Q, "C4"},  // Q -> C4
        {SDLK_W, "D4"},  // W -> D4
        {SDLK_E, "E4"},  // E -> E4
        {SDLK_R, "F4"},  // R -> F4
        {SDLK_T, "G4"},  // T -> G4
        {SDLK_Z, "A4"},  // Z -> A4
        {SDLK_U, "B4"},  // U -> B4
        {SDLK_I, "C5"},  // I -> C5
        {SDLK_O, "D5"},  // O -> D5
        {SDLK_P, "E5"}   // P -> E5
    };
}

// Global helper functions
void InitializePiano() {
    if (!gPiano) {
        gPiano = new Piano();
        gPiano->initialize();
    }
}

void ShutdownPiano() {
    if (gPiano) {
        delete gPiano;
        gPiano = nullptr;
    }
}