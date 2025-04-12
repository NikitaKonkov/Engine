#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_events.h>
#include <settings/settings.hpp>
#include <inputs/keyboard.hpp>


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
    if (!Engine::g_keyboard.loadConfiguration("resources/keyboard_config.txt")) {
        // Generate default keyboard config
        Engine::g_keyboard.mapAction("MOVE_FORWARD", SDLK_W, SDLK_UP);
        Engine::g_keyboard.mapAction("MOVE_BACKWARD", SDLK_S, SDLK_DOWN);
        Engine::g_keyboard.mapAction("MOVE_LEFT", SDLK_A, SDLK_LEFT);
        Engine::g_keyboard.mapAction("MOVE_RIGHT", SDLK_D, SDLK_RIGHT);
        Engine::g_keyboard.mapAction("JUMP", SDLK_SPACE);
        Engine::g_keyboard.mapAction("CROUCH", SDLK_LCTRL);
        Engine::g_keyboard.mapAction("RELOAD_SETTINGS", SDLK_F5);
        Engine::g_keyboard.saveConfiguration();
    }

    // Register callbacks for some actions
    Engine::g_keyboard.registerActionCallback("RELOAD_SETTINGS", 
        []() { // Press callback
            if (g_settings.loadFromFile(SETTINGS_FILE)) {
                SDL_Log("Settings refreshed");
            }
        }
    );

    // Now you can use g_settings throughout your application
    window = SDL_CreateWindow("Game Engine", 
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
            Engine::g_keyboard.handleEvent(event);
        }
        
        // Update keyboard states
        Engine::g_keyboard.update();
        
        // Example of checking action states
        if (Engine::g_keyboard.isActionPressed("MOVE_FORWARD")) {
            // Move character forward
            SDL_Log("Moving forward");
        }
        // Example of checking action states
        if (Engine::g_keyboard.isActionPressed("MOVE_BACKWARD")) {
            // Move character forward
            SDL_Log("Moving backward");
        }
        
        // Rendering code would go here
        
        // Window resizing example
        if (Engine::g_keyboard.isActionJustPressed("RELOAD_SETTINGS")) {
            SDL_SetWindowSize(window, g_settings.screenWidth, g_settings.screenHeight);
        }
    }
    
    // Clean up Vulkan resources (when implemented)
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}