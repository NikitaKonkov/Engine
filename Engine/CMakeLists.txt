#https://github.com/meemknight/cmakeSetup
#Version 1.1.0

cmake_minimum_required(VERSION 3.16)

# Define USE_AVX512 option before project() to ensure it's recognized
option(USE_AVX512 "Use AVX512 instructions if available (may not work on all machines)" OFF)

# Set CMake policies to handle deprecation warnings
if(POLICY CMP0115)
  cmake_policy(SET CMP0115 NEW)  # Require explicit file extensions for sources
endif()

# Get system core count and use half for build
include(ProcessorCount)
ProcessorCount(N)
if(N EQUAL 0)
  set(N 1)
endif()
math(EXPR N_HALF "${N} / 2")
if(N_HALF LESS 1)
  set(N_HALF 1)
endif()
set(CMAKE_BUILD_PARALLEL_LEVEL ${N_HALF} CACHE STRING "Set build parallelism to half available cores" FORCE)
message(STATUS "Building with ${N_HALF} parallel jobs (half of ${N} detected cores)")

set(CMAKE_CXX_STANDARD 17)
#! ! ! ! ! ! !
#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)
#delete the out folder after changing if visual studio doesn recognize the change!


set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


#may give problems on linux
if(PRODUCTION_BUILD)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization
else()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()



if(MSVC)
    # Default to AVX2 which has better compatibility
    if(USE_AVX512)
        message(STATUS "Building with AVX512 support - this requires compatible CPU hardware")
        # Apply directly to target instead of using directory-level options
        set(AVX_FLAGS "/arch:AVX512")
    else()
        message(STATUS "Building with AVX2 support for better compatibility")
        set(AVX_FLAGS "/arch:AVX2")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Add CPU optimization options for GCC/Clang compilers
    
    if(USE_AVX512)
        message(STATUS "Building with AVX512 support - this requires compatible CPU hardware")
        set(AVX_FLAGS "-mavx512f -mavx512vl -mavx512bw -mavx512dq")
    else()
        message(STATUS "Building with AVX2 support for better compatibility")
        set(AVX_FLAGS "-mavx2")
    endif()
endif()


project(gameengine)


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Add the path to the thirdparty libraries
add_subdirectory(thirdparty/sdl3-3.2.10)		#window oppener
# The following libraries are commented out since we're only using SDL3
# add_subdirectory(thirdparty/glfw-3.3.2)			#window oppener
# add_subdirectory(thirdparty/glad)				#opengl loader
# add_subdirectory(thirdparty/stb_image)			#loading immaged
# add_subdirectory(thirdparty/stb_truetype)		#loading ttf files
# #add_subdirectory(thirdparty/enet-1.3.17)		#networking
# add_subdirectory(thirdparty/raudio)				#audio
# add_subdirectory(thirdparty/glm)				#math
# add_subdirectory(thirdparty/imgui-docking)		#ui
# #----------V-----------------------V------------#my libraries												
# add_subdirectory(thirdparty/safeSave)			#saving data, designed for gamedev. It also makes backups so the player won't loose saves.
# add_subdirectory(thirdparty/profilerLib)		#profiling, just a simpe library for measuring elapsed time
# add_subdirectory(thirdparty/gl2d)				#2D rendering library
# add_subdirectory(thirdparty/glui)				#ui library, usefull for making game menus


# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")


add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)



if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") 

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1) 
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=0) 

else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0) 
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=1) 

endif()



target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )


if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
	
	#set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug<$<CONFIG:Debug>:Debug>")
	#set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

endif()

#force remove unicode
if (WIN32)
	target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE /UUNICODE /U_UNICODE)
endif()


target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/headers/")
# target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/gameLayer/")
# target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/platform/")


# Link with SDL3 only since that's what we're using
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE SDL3::SDL3)

# Apply the AVX flags directly to the target
target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE ${AVX_FLAGS})
