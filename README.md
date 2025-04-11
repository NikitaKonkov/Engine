# C++ Game Engine with SDL3 and Vulkan

A modern C++ game engine built with SDL3 and Vulkan, providing a flexible foundation for game development with optimized performance.

## Features

- SDL3 integration for window management and input handling
- Vulkan rendering backend
- Settings management system with file persistence
- CPU optimization options (AVX2/AVX512)
- Cross-platform CMake build system

## Requirements

- C++17 compatible compiler
- CMake 3.16 or higher
- SDL3
- Vulkan SDK 1.4.309.0 or compatible version

## Building the Project

### Prerequisites

1. Install [CMake](https://cmake.org/download/)
2. Install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
3. Set up SDL3 development libraries

### Quick Build Instructions

#### Windows
Use the provided batch script:
```
build.bat                 # Build with AVX2 (default)
build.bat --avx512        # Build with AVX512
build.bat --clean         # Clean build with AVX2
build.bat --clean --avx512  # Clean build with AVX512
```

#### Linux/macOS
Use the provided shell script:
```
./build.sh                # Build with AVX2 (default)
./build.sh --avx512       # Build with AVX512
./build.sh --clean        # Clean build with AVX2
./build.sh --clean --avx512  # Clean build with AVX512
```

### Manual Build Instructions (Alternative)

1. Clone the repository
   ```
   git clone https://github.com/NikitaKonkov/Game-Engine-CPP-SDL.git
   cd Game-Engine-CPP-SDL
   ```

2. Create a build directory
   ```
   mkdir build
   cd build
   ```

3. Configure the project
   ```
   cmake ..
   ```

4. Build the project
   ```
   cmake --build .
   ```

### Build Options

- `USE_AVX512` - Enable AVX512 instruction set (OFF by default, requires compatible hardware)
- `PRODUCTION_BUILD` - Configure for production release (OFF by default)

Example:
```
cmake -DUSE_AVX512=ON -DPRODUCTION_BUILD=ON ..
```

## Project Structure

- `/Engine` - Core engine code
  - `/include` - Header files
    - `/settings` - Settings management system
  - `/src` - Source files
    - `renderer.cpp` - Main rendering implementation
    - `settings.cpp` - Settings implementation
  - `/resources` - Game resources and configurations

## Configuration

The engine uses a settings system that manages configuration through a text file. Default settings include:

- Screen width/height
- Fullscreen mode
- VSync
- Maximum FPS
- Audio volume

Settings are automatically saved to and loaded from `resources/settings.txt`.

## License

[Your license information here]

## Contributing

[Your contribution guidelines here]