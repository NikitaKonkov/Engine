#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
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
    
    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            break;
        }
        
        // Vulkan rendering would happen here
        // For now, the window will just be blank
    }
    
    // Clean up Vulkan resources (when implemented)
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}