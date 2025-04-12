#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_events.h>
#include <settings/settings.hpp>
#include <Inputs/keyboard.hpp>

const std::string GRAPHICS_CONFIG_FILE = "resources/video_settings.txt";
const std::string KEYBOARD_CONFIG_FILE = "resources/keyboard_config.txt";
int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event event;
    
    // Initialize SDL and set log priority to show all messages
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }
    
    // Set log level to see all log messages
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    SDL_Log("SDL initialized successfully");
    


    // // Configure keyboard actions
    // if (!Keyboard::Input.loadConfiguration(KEYBOARD_CONFIG_FILE)) {
    //     // Generate default keyboard config
    //     Keyboard::Input.mapAction("MOVE_FORWARD", SDLK_W, SDLK_UP);
    //     Keyboard::Input.mapAction("MOVE_BACKWARD", SDLK_S, SDLK_DOWN);
    //     Keyboard::Input.mapAction("MOVE_LEFT", SDLK_A, SDLK_LEFT);
    //     Keyboard::Input.mapAction("MOVE_RIGHT", SDLK_D, SDLK_RIGHT);
    //     Keyboard::Input.mapAction("JUMP", SDLK_SPACE);
    //     Keyboard::Input.mapAction("CROUCH", SDLK_LCTRL);
    //     Keyboard::Input.mapAction("RELOAD_SETTINGS", SDLK_F5);
    //     Keyboard::Input.saveConfiguration();
    // }


    // Load preferences
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
                SDL_Log("Quit event received, exiting...");
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
        if (Keyboard::Input.isActionPressed("RELOAD_SETTINGS")) {
            g_settings.loadFromFile(GRAPHICS_CONFIG_FILE);
            SDL_SetWindowSize(window, g_settings.screenWidth, g_settings.screenHeight);
            SDL_Log("Window resized to %d x %d", g_settings.screenWidth, g_settings.screenHeight);
        }
    }
    
    // Clean up Vulkan resources (when implemented)
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}