#include <assets/piano/piano.hpp>
#include <audio/mixer.hpp>
#include <audio/audio.hpp>
#include <iostream>
#include <fstream>

// Global piano instance
Piano* gPiano = nullptr;

Piano::Piano() : sustainMode(false), recording(false), playing(false), recordStartTime(0), playbackIndex(0) {
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
    // Handle playback of recorded notes if we're in playback mode
    if (playing && !recordedNotes.empty()) {
        uint64_t currentTime = SDL_GetTicks();
        uint64_t elapsedTime = currentTime - recordStartTime;
        
        // Check if we need to play the next note
        while (playbackIndex < recordedNotes.size()) {
            const NoteRecord& record = recordedNotes[playbackIndex];
            
            // Only play the note if we've reached its timestamp
            if (elapsedTime >= record.timestamp) {
                playNote(record.note, record.duration);
                playbackIndex++;
            } else {
                break; // Not time for the next note yet
            }
        }
        
        // Check if we've finished playback
        if (playbackIndex >= recordedNotes.size()) {
            playing = false;
            std::cout << "Playback finished" << std::endl;
        }
    } else {
        SDL_Delay(10); // Small delay for the update loop when idle
    }
}

void Piano::handleKeyEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event.key.key;
        
        // Recording control - 'S' to save recording
        if (key == SDLK_S) {
            saveRecording();
            return;
        }
        
        // Playback control - 'D' to play recording
        if (key == SDLK_D) {
            playRecording();
            return;
        }
        
        // Check if this key is mapped to a piano note
        auto noteIt = keyToNoteMap.find(key);
        if (noteIt != keyToNoteMap.end()) {
            std::string note = noteIt->second;
            playNote(note);
            
            // Record the note if we're in recording mode
            if (recording) {
                uint64_t currentTime = SDL_GetTicks();
                uint64_t relativeTime = currentTime - recordStartTime;
                
                NoteRecord record;
                record.note = note;
                record.duration = 1000; // Default duration
                record.timestamp = relativeTime;
                
                recordedNotes.push_back(record);
                std::cout << "Recorded note: " << note << " at time: " << relativeTime << "ms" << std::endl;
            }
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

void Piano::startRecording() {
    // Clear previous recording
    recordedNotes.clear();
    recording = true;
    recordStartTime = SDL_GetTicks();
    std::cout << "Recording started" << std::endl;
}

void Piano::stopRecording() {
    if (recording) {
        recording = false;
        std::cout << "Recording stopped (" << recordedNotes.size() << " notes recorded)" << std::endl;
    }
}

void Piano::saveRecording() {
    if (recording) {
        stopRecording(); // Stop recording if currently active
    }
    
    if (recordedNotes.empty()) {
        std::cout << "No notes to save" << std::endl;
        startRecording(); // Start new recording
        return;
    }
    
    std::cout << "Recording saved with " << recordedNotes.size() << " notes" << std::endl;
    std::cout << "Press 'D' to play back the recording" << std::endl;
    
    startRecording(); // Start a new recording session
}

void Piano::playRecording() {
    if (recordedNotes.empty()) {
        std::cout << "No recording to play" << std::endl;
        return;
    }
    
    if (recording) {
        stopRecording(); // Stop recording if we're currently recording
    }
    
    playing = true;
    playbackIndex = 0;
    recordStartTime = SDL_GetTicks(); // Reset the start time for playback
    std::cout << "Playing back recording with " << recordedNotes.size() << " notes" << std::endl;
}

bool Piano::isRecording() const {
    return recording;
}

bool Piano::isPlaying() const {
    return playing;
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
        
        // Start recording by default when piano is initialized
        gPiano->startRecording();
    }
}

void ShutdownPiano() {
    if (gPiano) {
        delete gPiano;
        gPiano = nullptr;
    }
}