#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include <renderer/renderer.hpp>

class Application {
public:
    // Constructor with application title and window dimensions
    Application(const std::string& title, int width, int height);
    
    // Destructor for cleanup
    ~Application();
    
    // Initialize the application
    bool initialize();
    
    // Run the application main loop
    void run();
    
    // Process input events
    void processEvents();
    
    // Update application state
    void update();
    
    // Render the current frame
    void render();

private:
    // Window properties
    std::string title;
    int width;
    int height;
    
    // SDL window handle
    SDL_Window* window;
    
    // Renderer instance
    std::unique_ptr<Renderer> renderer;
    
    // Application state
    bool running;
};