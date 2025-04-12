#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_events.h>
#include <settings/settings.hpp>


int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event event;
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }
    
    // Early in your initialization
    bool settingsLoaded = g_settings.loadFromFile(std::string("resources/settings.txt"));
    if (!settingsLoaded) {
        // If loading failed, use defaults and save them
        g_settings = getDefaultSettings();
        g_settings.saveToFile(std::string("resources/settings.txt"));
    }

    // Now you can use g_settings throughout your application
    window = SDL_CreateWindow("Game Engine", 
                             g_settings.screenWidth, 
                             g_settings.screenHeight, 
                             SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        return 3;
    }
    
    // Here you would initialize Vulkan
    // This includes instance creation, device selection, swap chain setup, etc.
    // For now, we'll just have an empty window loop
    
    // In your event handling code
    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            break;
        }
        else if (event.type == SDL_EVENT_KEY_DOWN) {
            // Reload settings on F5 press
            if (event.key.key == SDLK_F5) {  // Changed from event.key.keysym.sym to event.key.key
                if (g_settings.loadFromFile("resources/settings.txt")) {
                    SDL_SetWindowSize(window, g_settings.screenWidth, g_settings.screenHeight);
                    // Apply other settings as needed
                    SDL_Log("Settings refreshed");
                }
            }
        }
        
        // Rendering code...
    }
    
    // Clean up Vulkan resources (when implemented)
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}