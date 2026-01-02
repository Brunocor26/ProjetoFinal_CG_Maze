cmake_minimum_required(VERSION 3.16)
project(maze C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Helper for Apple Silicon / Homebrew
list(APPEND CMAKE_PREFIX_PATH /opt/homebrew /opt/homebrew/lib/cmake)

# Dependencies
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

# ==========================================
# Shared source files
# ==========================================
set(COMMON_SOURCES
    src/Game.cpp
    src/Maze.cpp
    src/network.cpp
    src/TextRenderer.cpp
    src/glad.c
    include/kruksal/kruksal.cpp
    include/kruksal/maze_generator.cpp
)

# ==========================================
# Target: maze_host.exe (HOST MODE)
# ==========================================
add_executable(maze_host
    src/main_host.cpp
    ${COMMON_SOURCES}
)

set_target_properties(maze_host PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_NAME "maze_host.exe"
)

# ==========================================
# Target: maze_client.exe (CLIENT MODE)
# ==========================================
add_executable(maze_client
    src/main_client.cpp
    ${COMMON_SOURCES}
)

set_target_properties(maze_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_NAME "maze_client.exe"
)

# Configure the root directory header (for finding assets)
configure_file(configuration/root_directory.h.in configuration/root_directory.h)

# Include directories (apply to both targets)
foreach(target maze_host maze_client)
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/configuration
        ${FREETYPE_INCLUDE_DIRS}
    )
    
    # Libraries Linking
    target_link_libraries(${target} PRIVATE dl)
    target_link_libraries(${target} PRIVATE glfw)
    target_link_libraries(${target} PRIVATE ${FREETYPE_LIBRARIES})
    
    if (APPLE)
        target_compile_definitions(${target} PRIVATE GL_SILENCE_DEPRECATION)
        target_link_libraries(${target} PRIVATE "-framework OpenGL")
    elseif (WIN32)
        target_link_libraries(${target} PRIVATE opengl32)
    else()
        find_package(OpenGL REQUIRED)
        target_link_libraries(${target} PRIVATE OpenGL::GL)
    endif()
endforeach()

# ==========================================
# Custom Commands (Assets & Shaders)
# ==========================================

# 1. Copy the SHADERS folder for both targets
add_custom_command(TARGET maze_host POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    ${CMAKE_BINARY_DIR}/shaders
    COMMENT "Copying shaders directory..."
)

# 2. Copy the ASSETS folder (Recursive)
add_custom_command(TARGET maze_host POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets directory..."
)