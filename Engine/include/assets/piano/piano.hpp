#pragma once

#include <SDL3/SDL.h>
#include <map>
#include <string>
#include <functional>
#include <vector>
#include <chrono>

// Record entry structure to store note information
struct NoteRecord {
    std::string note;
    int duration;
    uint64_t timestamp;
};

class Piano {
public:
    Piano();
    ~Piano();

    // Initialize the piano system
    bool initialize();
    
    // Update and handle piano input
    void update();
    
    // Handle piano-specific key events
    void handleKeyEvent(const SDL_Event& event);
    
    // Piano controls
    void playNote(const std::string& note, int durationMs = 1000);
    void toggleSustainMode();
    void stopAllNotes();
    
    // Check if sustain mode is enabled
    bool isSustainModeEnabled() const;

    // Recording functionality
    void startRecording();
    void stopRecording();
    void saveRecording();
    void playRecording();
    bool isRecording() const;
    bool isPlaying() const;

private:
    // Map of notes to their frequencies
    std::map<std::string, float> noteFrequencies;
    
    // Sustain mode flag
    bool sustainMode;
    
    // Maps SDL keycodes to note names
    std::map<SDL_Keycode, std::string> keyToNoteMap;
    
    // Recording data
    std::vector<NoteRecord> recordedNotes;
    bool recording;
    bool playing;
    uint64_t recordStartTime;
    size_t playbackIndex;
    
    // Initialize the note to frequency mappings
    void initializeNoteFrequencies();
    
    // Initialize key to note mappings
    void initializeKeyMappings();
};

// Global piano instance that can be accessed from anywhere
extern Piano* gPiano;

// Helper functions for global piano access
void InitializePiano();
void ShutdownPiano();