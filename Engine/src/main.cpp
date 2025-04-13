#include <app/app.hpp>
#include <settings/settings.hpp>
#include <iostream>
#include <audio/audio.hpp>

int main(int argc, char* argv[]) {

    try {
        // Create application with settings from global config
        App app("Vulkan SDL Game Engine", 
                       g_settings.screenWidth, 
                       g_settings.screenHeight);
        
        // Initialize the application
        if (!app.initialize()) {
            std::cerr << "Application initialization failed!" << std::endl;
            return EXIT_FAILURE;
        }
        
        // Run the main loop
        app.run();
        
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}