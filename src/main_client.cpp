/**
 * @file main_client.cpp
 * @brief Main entry point for the CLIENT application
 * @author Project CG - Maze Game
 * @date 2025
 */

#include "../include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>

#include "../include/Game.h"

// Global Settings
const unsigned int SCR_WIDTH = 800;  ///< Default window width
const unsigned int SCR_HEIGHT = 600; ///< Default window height

// Global Game instance - CLIENT MODE
Game MazeGame(SCR_WIDTH, SCR_HEIGHT, GameMode::CLIENT);

// time variables
float deltaTime = 0.0f; ///< Time between current frame and last frame
float lastFrame = 0.0f; ///< Time of last frame

// Mouse State
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// Callback functions
/**
 * @brief GLFW callback for window resize events
 * @param window The window that received the event
 * @param width The new window width
 * @param height The new window height
 */
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

/**
 * @brief GLFW callback for key events
 * @param window The window that received the event
 * @param key The keyboard key that was pressed or released
 * @param scancode The system-specific scancode of the key
 * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT
 * @param mode Bit field describing which modifier keys were held down
 */
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mode);

/**
 * @brief GLFW callback for mouse movement events
 * @param window The window that received the event
 * @param xpos The new cursor x-coordinate, relative to the left edge of the
 * content area
 * @param ypos The new cursor y-coordinate, relative to the top edge of the
 * content area
 */
void mouse_callback(GLFWwindow *window, double xpos, double ypos);

int main() {
  // initialize and set up GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Fix for MacOS
#endif

  // create window
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Maze CLIENT", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // register callbacks
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // load GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // OpenGL Global settings
  glEnable(GL_DEPTH_TEST); // essencial for 3D

  // initialize game resources (Shaders, Models, Maze)
  MazeGame.Init();

  // Store window pointer in Game for pause functionality
  MazeGame.windowPtr = window;

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // time management
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // System events (inputs)
    glfwPollEvents();

    // game logic
    MazeGame.ProcessInput(deltaTime);
    MazeGame.Update(deltaTime);

    // rendering
    // clean color and depth buffer
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Sky blue background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    MazeGame.Render();

    // swap buffers
    glfwSwapBuffers(window);
  }

  // clean
  glfwTerminate();
  return 0;
}

// ---------------------------------------------------------------------------------------------
// IMPLEMENTAÇÃO DOS CALLBACKS
// ---------------------------------------------------------------------------------------------

// Ajusta o Viewport quando a janela é redimensionada
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

// Captura as teclas e guarda no array de estado do Jogo
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mode) {
  // ESC is now handled in Game::ProcessInput() for pause functionality
  // Removed: glfwSetWindowShouldClose(window, true);

  // Atualizar o array de teclas da classe Game
  if (key >= 0 && key < 1024) {
    if (action == GLFW_PRESS)
      MazeGame.Keys[key] = true;
    else if (action == GLFW_RELEASE)
      MazeGame.Keys[key] = false;
  }
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset =
      lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  MazeGame.ProcessMouseMovement(xoffset, yoffset);
}
