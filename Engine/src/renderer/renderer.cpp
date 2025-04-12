#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_events.h>
#include <settings/settings.hpp>
#include <Inputs/keyboard.hpp>


const std::string SETTINGS_FILE = "resources/settings.txt";

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event event;
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }
    
    // Early in your initialization
    bool settingsLoaded = g_settings.loadFromFile(SETTINGS_FILE);
    if (!settingsLoaded) {
        // If loading failed, use defaults and save them
        g_settings = getDefaultSettings();
        g_settings.saveToFile(SETTINGS_FILE);
    }

    // Configure keyboard actions
    if (!Keyboard::Input.loadConfiguration("resources/keyboard_config.txt")) {
        // Generate default keyboard config
        Keyboard::Input.mapAction("MOVE_FORWARD", SDLK_W, SDLK_UP);
        Keyboard::Input.mapAction("MOVE_BACKWARD", SDLK_S, SDLK_DOWN);
        Keyboard::Input.mapAction("MOVE_LEFT", SDLK_A, SDLK_LEFT);
        Keyboard::Input.mapAction("MOVE_RIGHT", SDLK_D, SDLK_RIGHT);
        Keyboard::Input.mapAction("JUMP", SDLK_SPACE);
        Keyboard::Input.mapAction("CROUCH", SDLK_LCTRL);
        Keyboard::Input.mapAction("RELOAD_SETTINGS", SDLK_F5);
        Keyboard::Input.saveConfiguration();
    }

    // Register callbacks for some actions
    Keyboard::Input.registerActionCallback("RELOAD_SETTINGS", 
        []() { // Press callback
            if (g_settings.loadFromFile(SETTINGS_FILE)) {
                SDL_Log("Settings refreshed");
            }
        }
    );

    // Now you can use g_settings throughout your application
    window = SDL_CreateWindow("Game Keyboard", 
                             g_settings.screenWidth, 
                             g_settings.screenHeight, 
                             SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        return 3;
    }
    
    // Main loop
    bool running = true;
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            
            // Pass events to keyboard manager
            Keyboard::Input.handleEvent(event);
        }
        
        // Update keyboard states
        Keyboard::Input.update();
        
        // Example of checking action states
        if (Keyboard::Input.isActionPressed("MOVE_FORWARD")) {
            // Move character forward
            SDL_Log("Moving forward");
        }
        // Example of checking action states
        if (Keyboard::Input.isActionPressed("MOVE_BACKWARD")) {
            // Move character forward
            SDL_Log("Moving backward");
        }
        
        // Rendering code would go here
        
        // Window resizing example
        if (Keyboard::Input.isActionJustPressed("RELOAD_SETTINGS")) {
            SDL_SetWindowSize(window, g_settings.screenWidth, g_settings.screenHeight);
        }
    }
    
    // Clean up Vulkan resources (when implemented)
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}