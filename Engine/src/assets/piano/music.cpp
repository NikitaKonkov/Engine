#include <audio/mixer.hpp>
#include <audio/audio.hpp>

void example(){

    if (event.type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event.key.key;

    if ()
    // Example usage of the AudioMixer and AudioSystem classes
    AudioMixer mixer;
    mixer.Initialize();
    
    // Play a sound with a frequency of 440Hz for 1000ms
    int channelId = mixer.PlaySound(440.0f, 1000);
    
    // Stop the sound after 500ms
    SDL_Delay(500);
    mixer.StopSound(channelId);
    
    // Shutdown the mixer
    mixer.StopAllSounds();
    // Clean up the global mixer instance

}