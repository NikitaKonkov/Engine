#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_events.h>
#include <settings/settings.hpp>
#include <Inputs/keyboard.hpp>
#include <config/resource_paths.hpp>

#include <memory>
#include <vector>

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
    

    // Load preferences
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
        // For now, we'll just log that we'd be rendering the shapes
        // SDL_Log("Would render %zu shapes", shapes.size());
        
        // Window resizing example
        if (Keyboard::Input.isActionPressed("RELOAD_SETTINGS")) {
            g_settings.loadFromFile(Config::GRAPHICS_CONFIG_FILE);
            SDL_SetWindowSize(window, g_settings.screenWidth, g_settings.screenHeight);
            SDL_Log("Window resized to %d x %d", g_settings.screenWidth, g_settings.screenHeight);
        }
    }
    
    // Clean up resources
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}