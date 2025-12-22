cmake_minimum_required(VERSION 3.16)
project(maze C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Helper for Apple Silicon / Homebrew
list(APPEND CMAKE_PREFIX_PATH /opt/homebrew /opt/homebrew/lib/cmake)

# Dependencies
find_package(glfw3 REQUIRED)

# ==========================================
# Target: maze.exe
# ==========================================
# We include the .cpp files found in include/kruksal here so they compile
add_executable(maze
    src/main.cpp
    src/Game.cpp
    src/Maze.cpp
    src/Objloader.cpp
    src/glad.c                      # Ensure this file exists!
    include/kruksal/kruksal.cpp
    include/kruksal/maze_generator.cpp
)

# Configure the root directory header (for finding assets)
configure_file(configuration/root_directory.h.in configuration/root_directory.h)

# Include directories
target_include_directories(maze PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/configuration
)

# Libraries Linking
target_link_libraries(maze PRIVATE dl)
target_link_libraries(maze PRIVATE glfw)

if (APPLE)
    target_compile_definitions(maze PRIVATE GL_SILENCE_DEPRECATION)
    target_link_libraries(maze PRIVATE "-framework OpenGL")
elseif (WIN32)
    target_link_libraries(maze PRIVATE opengl32)
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(maze PRIVATE OpenGL::GL)
endif()

set_target_properties(maze PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_NAME "maze.exe"
)

# ==========================================
# Custom Commands (Assets & Shaders)
# ==========================================

# 1. Copy the SHADERS folder
add_custom_command(TARGET maze POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    ${CMAKE_BINARY_DIR}/shaders
    COMMENT "Copying shaders directory..."
)

# 2. Copy the ASSETS folder (Recursive)
# This preserves the folder structure (models/trees9, etc.) so textures load correctly
add_custom_command(TARGET maze POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets directory..."
)