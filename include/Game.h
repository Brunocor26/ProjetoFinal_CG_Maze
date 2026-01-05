/**
 * @file Game.h
 * @brief Declaration of the Game class - core of the 3D maze game
 * @author Project CG - Maze Game
 * @date 2025
 */

#ifndef GAME_H
#define GAME_H

#include "../include/learnopengl/camera.h"
#include "Maze.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

/// Forward declaration of GLFW window
struct GLFWwindow;

// ============================================================================
// ENUMS
// ============================================================================

/**
 * @brief Available game modes
 *
 * HOST: Server that unlocks the client upon reaching the portal
 * CLIENT: Client that remains locked until the HOST reaches the portal
 */
enum class GameMode {
  HOST,  ///< Server/Host mode
  CLIENT ///< Client mode
};

// ============================================================================
// GAME CLASSE
// ============================================================================

/**
 * @brief Main class that manages the entire maze game
 *
 * This class is responsible for:
 * - Resource management (shaders, textures, meshes)
 * - Input processing (keyboard, mouse)
 * - Game logic (collisions, portal proximity)
 * - Network communication (HOST/CLIENT mode)
 * - Rendering (maze, outdoor environment, UI)
 *
 * The game supports two modes:
 * - HOST: Creates server and unlocks client when reaching the portal
 * - CLIENT: Connects to server and awaits unlock
 */
class Game {
public:
  // ========================================================================
  // GAME STATE
  // ========================================================================

  /// Key state array (true = pressed)
  bool Keys[1024];

  /// Window width in pixels
  unsigned int Width;

  /// Window height in pixels
  unsigned int Height;
  
  // Storage for windowed mode dimensions
  unsigned int WindowedWidth;
  unsigned int WindowedHeight;

  // ========================================================================
  // NETWORKING
  // ========================================================================

  /// Current game mode (HOST or CLIENT)
  GameMode mode;

  /// Indicates if movement is locked (CLIENT only)
  bool movementLocked;

  /// Server socket (HOST only - for accepting connections)
  int serverSocket;

  /// Connected client socket (HOST only)
  int clientSocket;

  /// Network socket (CLIENT only - connection to server)
  int networkSocket;

  /// Indicates if the portal has been reached
  bool connectedToPortal;

  /// 3D position of the portal in the maze
  glm::vec3 portalPosition;

  /// Color tint inherited from HOST (CLIENT only)
  glm::vec3 inheritedColorTint;

  /// Host IP address for client connections
  std::string hostIP;

  // ========================================================================
  // GAME RESOURCES
  // ========================================================================

  /// Pointer to the current maze
  Maze *currentMaze;

  /// Player camera (first person)
  Camera *camera;

  /// Outdoor ground mesh (grass)
  Mesh *outdoorGroundMesh;

  /// Tree mesh
  Mesh *treeMesh;

  /// Positions of all trees in the scene
  std::vector<glm::vec3> treePositions;

  /// Mesh of the portal at the end of the maze
  Mesh *gateMesh;

  /// Text rendering system
  class TextRenderer *textRenderer;

  // ========================================================================
  // UI STATE
  // ========================================================================

  /// Indicates if the intro dialog is showing
  bool showingIntroDialog;

  /// Indicates if the game is paused
  bool isPaused;

  /// Pointer to the window (for cursor control)
  GLFWwindow *windowPtr;

  // ========================================================================
  // CONSTRUCTOR AND DESTRUCTOR
  // ========================================================================

  /**
   * @brief Game constructor
   * @param width Window width in pixels
   * @param height Window height in pixels
   * @param gameMode Game mode (HOST or CLIENT), default is HOST
   * @param hostIP IP address of the host (used in CLIENT mode), default is "127.0.0.1"
   */
  Game(unsigned int width, unsigned int height,
       GameMode gameMode = GameMode::HOST,
       const std::string &hostIP = "127.0.0.1");

  /**
   * @brief Destructor - frees all allocated resources
   */
  ~Game();

  // ========================================================================
  // GAME LIFECYCLE
  // ========================================================================

  /**
   * @brief Initializes all game resources
   *
   * Loads shaders, 3D models, textures, generates the maze,
   * initializes the network and prepares the game for rendering.
   */
  void Init();

  /**
   * @brief Processes keyboard input
   *
   * Handles keys for movement (WASD/arrows), pause (ESC),
   * fullscreen (F) and dialog interaction (ENTER).
   *
   * @param dt Delta time for framerate-independent movement
   */
  void ProcessInput(float dt);

  /**
   * @brief Processes mouse movement
   *
   * Updates camera orientation based on mouse movement.
   *
   * @param xoffset Horizontal mouse offset
   * @param yoffset Vertical mouse offset
   * @param constrainPitch Limits vertical rotation (prevents flip)
   */
  void ProcessMouseMovement(float xoffset, float yoffset,
                            bool constrainPitch = true);

  /**
   * @brief Updates game logic
   *
   * Handles network communication (accept connections, receive messages)
   * and checks portal proximity.
   *
   * @param dt Delta time since the last frame
   */
  void Update(float dt);

  /**
   * @brief Renders the entire scene
   *
   * Draws the maze, outdoor environment, portal and UI overlays.
   */
  void Render();

  // ========================================================================
  // HELPER METHODS
  // ========================================================================

  /**
   * @brief Checks if the player is near the portal
   *
   * When HOST reaches the portal, sends unlock message
   * to CLIENT and shows victory dialog.
   */
  void CheckPortalProximity();

  /**
   * @brief Calculates color tint based on portal proximity
   *
   * The closer to the portal, the more the ambient color shifts to
   * blue/cyan tones, creating a progressive visual effect.
   *
   * @return RGB vector with the calculated tint
   */
  glm::vec3 GetEnvironmentTint();

  /**
   * @brief Renders the intro dialog
   *
   * Shows semi-transparent overlay with game info,
   * controls and button to start.
   */
  void RenderIntroDialog();
  void RenderPauseOverlay();

  /**
   * @brief Handles window resize events
   * @param width New width
   * @param height New height
   */
  void Resize(unsigned int width, unsigned int height);

private:
  // ========================================================================
  // PRIVATE RESOURCES (OVERLAY)
  // ========================================================================

  /// Shader program for 2D dialog overlay
  unsigned int overlayShaderProgram;

  /// Overlay VAO (Vertex Array Object)
  unsigned int overlayVAO;

  /// Overlay VBO (Vertex Buffer Object)
  unsigned int overlayVBO;

  /// Flag indicating if overlay resources have been initialized
  bool overlayResourcesInitialized;

  /**
   * @brief Initializes OpenGL resources for the overlay
   *
   * Creates shader, VAO and VBO for dialog rendering.
   * Called automatically on first render.
   */
  void InitializeOverlayResources();

  /**
   * @brief Frees OpenGL overlay resources
   *
   * Deletes shader, VAO and VBO. Called in destructor.
   */
  void CleanupOverlayResources();

  // ========================================================================
  // MINIMAP RESOURCES
  // ========================================================================

  /**
   * @brief VAO (Vertex Array Object) for the minimap
   */
  unsigned int minimapVAO;

  /**
   * @brief VBO (Vertex Buffer Object) for the minimap
   */
  unsigned int minimapVBO;

  /**
   * @brief Shader used for solid color 2D rendering
   *
   * Used by the minimap to draw walls, player and background
   * without textures or complex lighting.
   */
  class Shader *simpleShader;

  /**
   * @brief Renders the 2D Minimap
   *
   * Draws a top-down representation of the maze in the
   * top-right corner of the screen, showing:
   * - Walls (Black)
   * - Player (Red)
   * - Background (Dark Gray)
   *
   * @note Uses 2D orthographic projection.
   */
  void RenderMinimap();
};

#endif // GAME_H