#ifndef RESOURCE_PATHS_HPP
#define RESOURCE_PATHS_HPP

#include <string>

namespace Config {
    // Configuration files
    const std::string GRAPHICS_CONFIG_FILE = "resources/video_settings.txt";
    const std::string KEYBOARD_CONFIG_FILE = "resources/keyboard_config.txt";
    
    // Shader files
    const std::string VERTEX_SHADER_PATH = "resources/shaders/shape_vert.spv";
    const std::string FRAGMENT_SHADER_PATH = "resources/shaders/shape_frag.spv";
    
    // Add more resource paths as needed
    
} // namespace Config

#endif // RESOURCE_PATHS_HPP