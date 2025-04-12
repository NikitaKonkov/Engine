#include "application.hpp"
#include <Inputs/keyboard.hpp>
#include <settings/settings.hpp>
#include <stdexcept>
#include <iostream>

Application::Application(const std::string& title, int width, int height)
    : title(title), width(width), height(height), window(nullptr), running(false) {
}

Application::~Application() {
    // Renderer is automatically cleaned up by the unique_ptr
    
    // Clean up SDL resources
    if (window) {
        SDL_DestroyWindow(window);
    }
    
    SDL_Quit();
}

bool Application::initialize() {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create window with Vulkan support
    window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    try {
        // Create the renderer
        renderer = std::make_unique<Renderer>(window);
    }
    catch (const std::exception& e) {
        std::cerr << "Renderer initialization failed: " << e.what() << std::endl;
        return false;
    }
    
    running = true;
    return true;
}

void Application::run() {
    while (running) {
        processEvents();
        update();
        render();
    }
}

void Application::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
        
        // Handle window events
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            // Handle window resizing if needed
            // In a more complete implementation, you would recreate the swapchain here
        }
        
        // Process keyboard input through your input system
        Keyboard::Input.handleEvent(event);
    }
}

void Application::update() {
    // Update input state
    Keyboard::Input.update();
    
    // Add game logic updates here
    // This is where you would update game state, physics, etc.
}

void Application::render() {
    // Let the renderer draw the current frame
    renderer->drawFrame();
}