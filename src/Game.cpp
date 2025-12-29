// ============================================================================
// MAZE GAME - Main Game Implementation
// ============================================================================
// This file contains the core game logic including:
// - Game initialization and cleanup
// - Input processing (keyboard, mouse)
// - Network communication (host/client mode)
// - Rendering (3D maze, UI overlays)
// - Collision detection and portal proximity
// ============================================================================

#include "../include/Game.h"
#include "../include/Network.h"
#include "../include/Shader.h"
#include "../include/TextRenderer.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "../include/Texture.h"
#include "../include/stb_image.h"
#include <iostream>

// ============================================================================
// GLOBAL CONSTANTS AND VARIABLES
// ============================================================================

// Network configuration
const int PORT = 8080;          // Server port for host/client communication
const char *HOST = "127.0.0.1"; // Localhost IP for client connection

// Global rendering resources (shared across game instances)
Shader *gameShader; // Main shader program for 3D rendering
Mesh *wall_mesh;    // Mesh for maze walls
Mesh *floor_mesh;   // Mesh for maze floor

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

/**
 * Loads a texture from the specified file path
 * @param path File path to the texture image
 * @return OpenGL texture ID
 */
unsigned int loadTexture(char const *path);

// ============================================================================
// CONSTRUCTOR & DESTRUCTOR
// ============================================================================

/**
 * Game Constructor
 * Initializes all game state variables and resources
 * @param width Window width in pixels
 * @param height Window height in pixels
 * @param gameMode Either HOST or CLIENT mode for networked gameplay
 */
Game::Game(unsigned int width, unsigned int height, GameMode gameMode)
    : Width(width), Height(height), currentMaze(nullptr), camera(nullptr),
      outdoorGroundMesh(nullptr), treeMesh(nullptr), gateMesh(nullptr),
      networkSocket(-1), connectedToPortal(false), portalPosition(0.0f),
      isPaused(false), windowPtr(nullptr), mode(gameMode),
      movementLocked(gameMode == GameMode::CLIENT), serverSocket(-1),
      clientSocket(-1), showingIntroDialog(true), textRenderer(nullptr),
      inheritedColorTint(1.0f, 1.0f, 1.0f), overlayShaderProgram(0),
      overlayVAO(0), overlayVBO(0), overlayResourcesInitialized(false) {
  // Initialize all keyboard keys to unpressed state
  for (int i = 0; i < 1024; i++)
    Keys[i] = false;
}

/**
 * Game Destructor
 * Cleans up all allocated resources including meshes, textures,
 * shaders, and network connections
 */
Game::~Game() {
  // Free game objects
  delete currentMaze;
  delete camera;
  delete gameShader;
  delete wall_mesh;
  delete floor_mesh;
  delete outdoorGroundMesh;
  delete treeMesh;
  delete gateMesh;
  delete textRenderer;

  // Clean up OpenGL overlay resources (VAO, VBO, shader)
  CleanupOverlayResources();

  // Close all network connections gracefully
  if (networkSocket >= 0) {
    Network::closeSocket(networkSocket);
  }
  if (serverSocket >= 0) {
    Network::closeSocket(serverSocket);
  }
  if (clientSocket >= 0) {
    Network::closeSocket(clientSocket);
  }
}

// ============================================================================
// GAME INITIALIZATION
// ============================================================================

/**
 * Initialize Game Resources
 * Sets up network connections, loads all assets (textures, models, shaders),
 * generates the maze, and prepares the game for rendering
 */
void Game::Init() {
  // ---------------------------------------------------------------------------
  // NETWORK SETUP - Initialize based on game mode (HOST or CLIENT)
  // ---------------------------------------------------------------------------

  if (mode == GameMode::HOST) {
    std::cout << "\n===== HOST MODE =====" << std::endl;
    // Create server socket
    serverSocket = Network::createSocket();
    if (serverSocket >= 0) {
      if (Network::bindAndListen(serverSocket, PORT)) { // if successful
        std::cout << "Server listening on port " << PORT << std::endl;
        std::cout << "Waiting for client connection..." << std::endl;
      } else {
        std::cerr << "Failed to start server" << std::endl;
      }
    }
  } else {
    std::cout << "\n===== CLIENT MODE =====" << std::endl;
    std::cout << "MOVEMENT LOCKED - Waiting for host to reach portal"
              << std::endl;
    // Create client socket
    networkSocket = Network::createSocket();
    if (networkSocket >= 0) {
      std::cout << "Connecting to host at " << HOST << ":" << PORT << "..."
                << std::endl;
      if (Network::connectToServer(networkSocket, HOST, PORT)) {
        std::cout << "Connected to host!" << std::endl;
      } else {
        std::cerr << "Failed to connect to host" << std::endl;
      }
    }
  }
  std::cout << "=====================\n" << std::endl;

  // Show intro dialog instructions
  std::cout << "\n╔════════════════════════════════════════════════╗"
            << std::endl;
  std::cout << "║        WELCOME TO THE MAZE GAME!              ║" << std::endl;
  std::cout << "╠════════════════════════════════════════════════╣"
            << std::endl;
  if (mode == GameMode::HOST) {
    std::cout << "║  MODE: HOST                                    ║"
              << std::endl;
    std::cout << "║  GOAL: Reach the portal to unlock the client  ║"
              << std::endl;
  } else {
    std::cout << "║  MODE: CLIENT                                  ║"
              << std::endl;
    std::cout << "║  GOAL: Wait for unlock, then reach portal     ║"
              << std::endl;
  }
  std::cout << "╠════════════════════════════════════════════════╣"
            << std::endl;
  std::cout << "║  CONTROLS:                                     ║"
            << std::endl;
  std::cout << "║  • WASD / Arrow Keys - Move                    ║"
            << std::endl;
  std::cout << "║  • Mouse - Look around                         ║"
            << std::endl;
  std::cout << "║  • ESC - Pause/Resume                          ║"
            << std::endl;
  std::cout << "╠════════════════════════════════════════════════╣"
            << std::endl;
  std::cout << "║  Press ENTER to start the game!               ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════╝\n"
            << std::endl;

  // 1. Camera - Start higher and looking down to ensure visibility
  camera = new Camera(glm::vec3(15.0f, 20.0f, 15.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -89.0f);

  // 2. Shaders
  gameShader = new Shader("shaders/maze_vs.glsl", "shaders/maze_fs.glsl");
  std::cout << "Shader Program ID: " << gameShader->ID << std::endl;
  gameShader->use();
  gameShader->setInt("texture1", 0);

  // 3. Geometry Data
  // Cube (Walls)
  std::vector<Vertex> wallVertices = {
      // Position           // Normal         // TexCoords
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},

      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

      {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

      {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
      {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}};

  // plane (Floor)
  std::vector<Vertex> floorVertices = {
      {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
      {{0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
      {{0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
      {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}};

  // Mesh creation moved to after texture loading
  // wall_mesh = new Mesh(wallVertices, {});
  // floor_mesh = new Mesh(floorVertices, {});

  // Load Textures
  // Walls use Bricks101 textures
  unsigned int wallTex = loadTexture("assets/textures/Bricks101_4K-PNG/"
                                     "Bricks101_4K-PNG_Color.png");
  unsigned int wallNormal = loadTexture("assets/textures/Bricks101_4K-PNG/"
                                        "Bricks101_4K-PNG_NormalGL.png");
  unsigned int wallRoughness = loadTexture("assets/textures/Bricks101_4K-PNG/"
                                           "Bricks101_4K-PNG_Roughness.png");

  // Floor uses PavingStones138 textures
  unsigned int floorTex = loadTexture("assets/textures/PavingStones138_4K-PNG/"
                                      "PavingStones138_4K-PNG_Color.png");
  unsigned int floorNormal =
      loadTexture("assets/textures/PavingStones138_4K-PNG/"
                  "PavingStones138_4K-PNG_NormalGL.png");
  unsigned int floorRoughness =
      loadTexture("assets/textures/PavingStones138_4K-PNG/"
                  "PavingStones138_4K-PNG_Roughness.png");

  // Create Texture structs for walls (Bricks)
  Texture wallTextureStruct;
  wallTextureStruct.id = wallTex;
  wallTextureStruct.type = "texture_diffuse";
  wallTextureStruct.path =
      "assets/textures/Bricks101_4K-PNG/Bricks101_4K-PNG_Color.png";

  Texture wallNormalStruct;
  wallNormalStruct.id = wallNormal;
  wallNormalStruct.type = "texture_normal";
  wallNormalStruct.path =
      "assets/textures/Bricks101_4K-PNG/Bricks101_4K-PNG_NormalGL.png";

  Texture wallRoughnessStruct;
  wallRoughnessStruct.id = wallRoughness;
  wallRoughnessStruct.type = "texture_roughness";
  wallRoughnessStruct.path =
      "assets/textures/Bricks101_4K-PNG/Bricks101_4K-PNG_Roughness.png";

  // Create Texture structs for floor (PavingStones)
  Texture floorTextureStruct;
  floorTextureStruct.id = floorTex;
  floorTextureStruct.type = "texture_diffuse";
  floorTextureStruct.path =
      "assets/textures/PavingStones138_4K-PNG/PavingStones138_4K-PNG_Color.png";

  Texture floorNormalStruct;
  floorNormalStruct.id = floorNormal;
  floorNormalStruct.type = "texture_normal";
  floorNormalStruct.path = "assets/textures/PavingStones138_4K-PNG/"
                           "PavingStones138_4K-PNG_NormalGL.png";

  Texture floorRoughnessStruct;
  floorRoughnessStruct.id = floorRoughness;
  floorRoughnessStruct.type = "texture_roughness";
  floorRoughnessStruct.path = "assets/textures/PavingStones138_4K-PNG/"
                              "PavingStones138_4K-PNG_Roughness.png";

  std::vector<Texture> wallTextures;
  wallTextures.push_back(wallTextureStruct);
  wallTextures.push_back(wallNormalStruct);
  wallTextures.push_back(wallRoughnessStruct);

  std::vector<Texture> floorTextures;
  floorTextures.push_back(floorTextureStruct);
  floorTextures.push_back(floorNormalStruct);
  floorTextures.push_back(floorRoughnessStruct);

  // 4. Maze (Pass textures to Mesh)
  wall_mesh = new Mesh(wallVertices, {}, wallTextures);
  floor_mesh = new Mesh(floorVertices, {}, floorTextures);

  currentMaze = new Maze(wall_mesh, floor_mesh);
  // currentMaze->wallTexture = wallTex; // REMOVED: Managed by Mesh now
  // currentMaze->floorTexture = floorTex; // REMOVED: Managed by Mesh now

  currentMaze->Generate(15, 15);

  // Find valid start position
  bool foundStart = false;
  for (int z = 0; z < currentMaze->height; z++) {
    for (int x = 0; x < currentMaze->width; x++) {
      if (currentMaze->grid[z][x] == 1) { // 1 is path
        delete camera;
        camera = new Camera(glm::vec3(x * currentMaze->cellSize, 0.5f,
                                      z * currentMaze->cellSize),
                            glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

        std::cout << "Start Position Found at: " << x << ", " << z << std::endl;
        foundStart = true;
        goto end_loop_init; // Break out of nested loop
      }
    }
  }
end_loop_init:
  if (!foundStart) {
    std::cout << "CRITICAL: No path (1) found in maze grid!" << std::endl;
  }

  // 5. Outdoor Environment
  // Create grass texture for outdoor ground
  unsigned int grassTex = loadTexture("assets/textures/Grass005_4K-PNG/"
                                      "Grass005_4K-PNG_Color.png");
  unsigned int grassNormal = loadTexture("assets/textures/Grass005_4K-PNG/"
                                         "Grass005_4K-PNG_NormalGL.png");
  unsigned int grassRoughness = loadTexture("assets/textures/Grass005_4K-PNG/"
                                            "Grass005_4K-PNG_Roughness.png");

  Texture grassTextureStruct;
  grassTextureStruct.id = grassTex;
  grassTextureStruct.type = "texture_diffuse";
  grassTextureStruct.path =
      "assets/textures/Grass005_4K-PNG/Grass005_4K-PNG_Color.png";

  Texture grassNormalStruct;
  grassNormalStruct.id = grassNormal;
  grassNormalStruct.type = "texture_normal";
  grassNormalStruct.path =
      "assets/textures/Grass005_4K-PNG/Grass005_4K-PNG_NormalGL.png";

  Texture grassRoughnessStruct;
  grassRoughnessStruct.id = grassRoughness;
  grassRoughnessStruct.type = "texture_roughness";
  grassRoughnessStruct.path =
      "assets/textures/Grass005_4K-PNG/Grass005_4K-PNG_Roughness.png";

  std::vector<Texture> grassTextures;
  grassTextures.push_back(grassTextureStruct);
  grassTextures.push_back(grassNormalStruct);
  grassTextures.push_back(grassRoughnessStruct);

  // Create large outdoor ground plane (extends 10 cells beyond maze)
  float mazeSize = currentMaze->width * currentMaze->cellSize;
  float groundMargin = 10.0f * currentMaze->cellSize; // 10 cells beyond maze
  float groundSize = mazeSize + 2 * groundMargin;
  float halfGroundSize = groundSize / 2.0f;
  float groundCenter = mazeSize / 2.0f;

  std::vector<Vertex> outdoorGroundVertices = {
      {{groundCenter - halfGroundSize, -0.01f, groundCenter - halfGroundSize},
       {0.0f, 1.0f, 0.0f},
       {0.0f, groundSize / 5.0f}},
      {{groundCenter + halfGroundSize, -0.01f, groundCenter - halfGroundSize},
       {0.0f, 1.0f, 0.0f},
       {groundSize / 5.0f, groundSize / 5.0f}},
      {{groundCenter + halfGroundSize, -0.01f, groundCenter + halfGroundSize},
       {0.0f, 1.0f, 0.0f},
       {groundSize / 5.0f, 0.0f}},
      {{groundCenter + halfGroundSize, -0.01f, groundCenter + halfGroundSize},
       {0.0f, 1.0f, 0.0f},
       {groundSize / 5.0f, 0.0f}},
      {{groundCenter - halfGroundSize, -0.01f, groundCenter + halfGroundSize},
       {0.0f, 1.0f, 0.0f},
       {0.0f, 0.0f}},
      {{groundCenter - halfGroundSize, -0.01f, groundCenter - halfGroundSize},
       {0.0f, 1.0f, 0.0f},
       {0.0f, groundSize / 5.0f}}};

  outdoorGroundMesh = new Mesh(outdoorGroundVertices, {}, grassTextures);

  // Create simple procedural tree (trunk + cone-shaped canopy)
  std::vector<Vertex> treeVertices;

  // Trunk (brown cylinder approximated with box)
  float trunkHeight = 2.0f;
  float trunkWidth = 0.3f;
  glm::vec3 brownColor(0.6f, 0.4f, 0.2f);

  // Trunk front face
  treeVertices.push_back(
      {{-trunkWidth, 0.0f, -trunkWidth}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}});
  treeVertices.push_back(
      {{trunkWidth, 0.0f, -trunkWidth}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}});
  treeVertices.push_back({{trunkWidth, trunkHeight, -trunkWidth},
                          {0.0f, 0.0f, -1.0f},
                          {1.0f, 1.0f}});
  treeVertices.push_back({{trunkWidth, trunkHeight, -trunkWidth},
                          {0.0f, 0.0f, -1.0f},
                          {1.0f, 1.0f}});
  treeVertices.push_back({{-trunkWidth, trunkHeight, -trunkWidth},
                          {0.0f, 0.0f, -1.0f},
                          {0.0f, 1.0f}});
  treeVertices.push_back(
      {{-trunkWidth, 0.0f, -trunkWidth}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}});

  // Canopy (simple pyramid/cone for leaves)
  float canopyHeight = 3.0f;
  float canopyBase = 1.5f;
  glm::vec3 greenColor(0.2f, 0.8f, 0.2f);

  // Canopy triangular faces
  glm::vec3 canopyTop(0.0f, trunkHeight + canopyHeight, 0.0f);
  glm::vec3 canopyCorner1(-canopyBase, trunkHeight, -canopyBase);
  glm::vec3 canopyCorner2(canopyBase, trunkHeight, -canopyBase);
  glm::vec3 canopyCorner3(canopyBase, trunkHeight, canopyBase);
  glm::vec3 canopyCorner4(-canopyBase, trunkHeight, canopyBase);

  // Front face
  treeVertices.push_back({canopyCorner1, {0.0f, 0.5f, -1.0f}, {0.0f, 0.0f}});
  treeVertices.push_back({canopyCorner2, {0.0f, 0.5f, -1.0f}, {1.0f, 0.0f}});
  treeVertices.push_back({canopyTop, {0.0f, 0.5f, -1.0f}, {0.5f, 1.0f}});

  // Right face
  treeVertices.push_back({canopyCorner2, {1.0f, 0.5f, 0.0f}, {0.0f, 0.0f}});
  treeVertices.push_back({canopyCorner3, {1.0f, 0.5f, 0.0f}, {1.0f, 0.0f}});
  treeVertices.push_back({canopyTop, {1.0f, 0.5f, 0.0f}, {0.5f, 1.0f}});

  // Back face
  treeVertices.push_back({canopyCorner3, {0.0f, 0.5f, 1.0f}, {0.0f, 0.0f}});
  treeVertices.push_back({canopyCorner4, {0.0f, 0.5f, 1.0f}, {1.0f, 0.0f}});
  treeVertices.push_back({canopyTop, {0.0f, 0.5f, 1.0f}, {0.5f, 1.0f}});

  // Left face
  treeVertices.push_back({canopyCorner4, {-1.0f, 0.5f, 0.0f}, {0.0f, 0.0f}});
  treeVertices.push_back({canopyCorner1, {-1.0f, 0.5f, 0.0f}, {1.0f, 0.0f}});
  treeVertices.push_back({canopyTop, {-1.0f, 0.5f, 0.0f}, {0.5f, 1.0f}});

  treeMesh = new Mesh(treeVertices, {}, {});

  // Position trees around the perimeter of the outdoor area
  float treeSpacing = 4.0f;

  // Trees along the outer edges
  for (float x = -groundMargin; x < mazeSize + groundMargin; x += treeSpacing) {
    // North edge
    treePositions.push_back(glm::vec3(x, 0.0f, -groundMargin));
    // South edge
    treePositions.push_back(glm::vec3(x, 0.0f, mazeSize + groundMargin));
  }

  for (float z = -groundMargin; z < mazeSize + groundMargin; z += treeSpacing) {
    // West edge
    treePositions.push_back(glm::vec3(-groundMargin, 0.0f, z));
    // East edge
    treePositions.push_back(glm::vec3(mazeSize + groundMargin, 0.0f, z));
  }

  std::cout << "Outdoor environment created with " << treePositions.size()
            << " trees" << std::endl;

  // 6. Create Simple Procedural Portal at Maze End
  std::vector<Vertex> portalVertices;

  // Portal dimensions
  float pillarWidth = 0.2f;
  float pillarHeight = 2.0f;
  float archWidth = 1.5f;
  float archThickness = 0.15f;

  glm::vec3 portalColor(0.5f, 0.3f, 0.8f); // Purple/mystical color

  // Left Pillar (simple box)
  float leftX = -archWidth / 2.0f;
  // Front face
  portalVertices.push_back({{leftX - pillarWidth, 0.0f, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {0.0f, 0.0f}});
  portalVertices.push_back({{leftX + pillarWidth, 0.0f, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {1.0f, 0.0f}});
  portalVertices.push_back({{leftX + pillarWidth, pillarHeight, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {1.0f, 1.0f}});
  portalVertices.push_back({{leftX + pillarWidth, pillarHeight, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {1.0f, 1.0f}});
  portalVertices.push_back({{leftX - pillarWidth, pillarHeight, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {0.0f, 1.0f}});
  portalVertices.push_back({{leftX - pillarWidth, 0.0f, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {0.0f, 0.0f}});

  // Back face
  portalVertices.push_back({{leftX - pillarWidth, 0.0f, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {0.0f, 0.0f}});
  portalVertices.push_back({{leftX + pillarWidth, 0.0f, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {1.0f, 0.0f}});
  portalVertices.push_back({{leftX + pillarWidth, pillarHeight, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {1.0f, 1.0f}});
  portalVertices.push_back({{leftX + pillarWidth, pillarHeight, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {1.0f, 1.0f}});
  portalVertices.push_back({{leftX - pillarWidth, pillarHeight, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {0.0f, 1.0f}});
  portalVertices.push_back({{leftX - pillarWidth, 0.0f, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {0.0f, 0.0f}});

  // Right Pillar
  float rightX = archWidth / 2.0f;
  // Front face
  portalVertices.push_back({{rightX - pillarWidth, 0.0f, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {0.0f, 0.0f}});
  portalVertices.push_back({{rightX + pillarWidth, 0.0f, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {1.0f, 0.0f}});
  portalVertices.push_back({{rightX + pillarWidth, pillarHeight, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {1.0f, 1.0f}});
  portalVertices.push_back({{rightX + pillarWidth, pillarHeight, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {1.0f, 1.0f}});
  portalVertices.push_back({{rightX - pillarWidth, pillarHeight, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {0.0f, 1.0f}});
  portalVertices.push_back({{rightX - pillarWidth, 0.0f, -pillarWidth},
                            {0.0f, 0.0f, -1.0f},
                            {0.0f, 0.0f}});

  // Back face
  portalVertices.push_back({{rightX - pillarWidth, 0.0f, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {0.0f, 0.0f}});
  portalVertices.push_back({{rightX + pillarWidth, 0.0f, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {1.0f, 0.0f}});
  portalVertices.push_back({{rightX + pillarWidth, pillarHeight, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {1.0f, 1.0f}});
  portalVertices.push_back({{rightX + pillarWidth, pillarHeight, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {1.0f, 1.0f}});
  portalVertices.push_back({{rightX - pillarWidth, pillarHeight, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {0.0f, 1.0f}});
  portalVertices.push_back({{rightX - pillarWidth, 0.0f, pillarWidth},
                            {0.0f, 0.0f, 1.0f},
                            {0.0f, 0.0f}});

  // Top arch (curved, made of segments)
  int archSegments = 12;
  float archRadius = archWidth / 2.0f;
  for (int i = 0; i < archSegments; i++) {
    float angle1 = glm::radians(180.0f - (i * 180.0f / archSegments));
    float angle2 = glm::radians(180.0f - ((i + 1) * 180.0f / archSegments));

    float x1 = archRadius * cos(angle1);
    float y1 = pillarHeight + archRadius * sin(angle1);
    float x2 = archRadius * cos(angle2);
    float y2 = pillarHeight + archRadius * sin(angle2);

    // Front face of arch segment
    portalVertices.push_back(
        {{x1, y1, -archThickness}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}});
    portalVertices.push_back(
        {{x2, y2, -archThickness}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}});
    portalVertices.push_back(
        {{x2, y2, archThickness}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}});
    portalVertices.push_back(
        {{x2, y2, archThickness}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}});
    portalVertices.push_back(
        {{x1, y1, archThickness}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}});
    portalVertices.push_back(
        {{x1, y1, -archThickness}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}});
  }

  gateMesh = new Mesh(portalVertices, {}, {});
  std::cout << "Procedural portal created successfully!" << std::endl;

  // Store portal position for proximity detection
  portalPosition =
      glm::vec3(currentMaze->endParams.x * currentMaze->cellSize, 0.0f,
                currentMaze->endParams.y * currentMaze->cellSize);

  // Initialize text renderer for intro dialog
  textRenderer = new TextRenderer(Width, Height);
  // Load font (TODO: adjust path)
  textRenderer->Load("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
  std::cout << "Text renderer initialized!" << std::endl;
}

/**
 * Load Texture Helper Function
 * Loads an image file and creates an OpenGL texture with mipmaps
 * @param path File path to the texture image
 * @return OpenGL texture ID
 */
unsigned int loadTexture(char const *path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  // Flip texture vertically to match OpenGL's coordinate system
  stbi_set_flip_vertically_on_load(true);

  // Load image data
  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

  if (data) {
    std::cout << "Texture Loaded! Path: " << path << " | W: " << width
              << " H: " << height << " Ch: " << nrComponents << std::endl;

    // Determine texture format based on number of color channels
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED; // Grayscale
    else if (nrComponents == 3)
      format = GL_RGB; // RGB
    else if (nrComponents == 4)
      format = GL_RGBA; // RGBA (with alpha)

    // Upload texture to GPU
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(
        GL_TEXTURE_2D); // Generate mipmap levels for better quality

    // Set texture wrapping and filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR); // Trilinear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

/**
 * Collision Detection Helper
 * Checks if a target position collides with maze walls
 * @param targetPos The 3D position to check
 * @param maze Pointer to the maze object
 * @return true if collision detected, false otherwise
 */
bool CheckCollision(glm::vec3 targetPos, Maze *maze) {
  if (!maze)
    return false;
  // Simple point collision (can be improved to bounding box in future)
  return maze->IsWall(targetPos.x, targetPos.z);
}

// ============================================================================
// INPUT PROCESSING
// ============================================================================

/**
 * Process Keyboard Input
 * Handles all keyboard inputs including dialog interactions, pause/resume,
 * fullscreen toggle, and player movement
 * @param dt Delta time since last frame (for frame-independent movement)
 */
void Game::ProcessInput(float dt) {
  // ---------------------------------------------------------------------------
  // INTRO DIALOG HANDLING
  // ---------------------------------------------------------------------------

  // Track ENTER key state to detect single press (not hold)
  static bool enterPressedLastFrame = false;
  bool enterPressed = Keys[GLFW_KEY_ENTER] || Keys[GLFW_KEY_KP_ENTER];

  if (showingIntroDialog) {
    // Dismiss intro dialog when ENTER is pressed
    if (enterPressed && !enterPressedLastFrame) {
      showingIntroDialog = false;
      std::cout << "\n=== Game Started! ===" << std::endl;
    }
    enterPressedLastFrame = enterPressed;
    return; // Block all other input while dialog is shown
  }
  enterPressedLastFrame = enterPressed;

  // ---------------------------------------------------------------------------
  // PAUSE/UNPAUSE HANDLING (ESC key)
  // ---------------------------------------------------------------------------
  static bool escPressedLastFrame = false;
  bool escPressed = Keys[GLFW_KEY_ESCAPE];

  if (escPressed && !escPressedLastFrame && windowPtr) {
    // Toggle pause
    isPaused = !isPaused;

    if (isPaused) {
      // Show cursor
      glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      std::cout << "Game PAUSED. Press ESC to resume." << std::endl;
    } else {
      // Hide cursor
      glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      std::cout << "Game RESUMED." << std::endl;
    }
  }
  escPressedLastFrame = escPressed;

  // ---------------------------------------------------------------------------
  // FULLSCREEN TOGGLE (F key)
  // ---------------------------------------------------------------------------
  static bool fPressedLastFrame = false;
  bool fPressed = Keys[GLFW_KEY_F];

  if (fPressed && !fPressedLastFrame && windowPtr) {
    GLFWmonitor *monitor = glfwGetWindowMonitor(windowPtr);

    if (monitor == nullptr) {
      // Currently in windowed mode - switch to fullscreen
      GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
      const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);
      glfwSetWindowMonitor(windowPtr, primaryMonitor, 0, 0, mode->width,
                           mode->height, mode->refreshRate);
      std::cout << "Switched to FULLSCREEN mode" << std::endl;
    } else {
      // Currently in fullscreen - switch to windowed mode
      glfwSetWindowMonitor(windowPtr, nullptr, 100, 100, Width, Height, 0);
      std::cout << "Switched to WINDOWED mode" << std::endl;
    }
  }
  fPressedLastFrame = fPressed;

  // ---------------------------------------------------------------------------
  // MOVEMENT RESTRICTIONS
  // ---------------------------------------------------------------------------

  // Don't process movement if game is paused
  if (isPaused) {
    return;
  }

  // CLIENT MODE: Block movement until host reaches portal and sends unlock
  if (mode == GameMode::CLIENT && movementLocked) {
    return;
  }

  // ---------------------------------------------------------------------------
  // PLAYER MOVEMENT (WASD/Arrow Keys)
  // ---------------------------------------------------------------------------

  // Get current player position and calculate movement velocity
  glm::vec3 currentPos = camera->Position;
  glm::vec3 nextPos = currentPos;
  float velocity = camera->MovementSpeed * dt; // Frame-independent movement

  // Calculate movement directions (lock to horizontal plane - no flying!)
  glm::vec3 front = camera->Front;
  front.y = 0.0f; // Remove vertical  component
  front = glm::normalize(front);
  glm::vec3 right = camera->Right;
  right.y = 0.0f; // Remove vertical component
  right = glm::normalize(right);

  // Accumulate movement from all pressed keys
  glm::vec3 proposedMove = glm::vec3(0.0f);

  // W or UP Arrow: Move forward
  if (Keys[GLFW_KEY_W] || Keys[GLFW_KEY_UP])
    proposedMove += front * velocity;
  // S or DOWN Arrow: Move backward
  if (Keys[GLFW_KEY_S] || Keys[GLFW_KEY_DOWN])
    proposedMove -= front * velocity;
  // A or LEFT Arrow: Strafe left
  if (Keys[GLFW_KEY_A] || Keys[GLFW_KEY_LEFT])
    proposedMove -= right * velocity;
  // D or RIGHT Arrow: Strafe right
  if (Keys[GLFW_KEY_D] || Keys[GLFW_KEY_RIGHT])
    proposedMove += right * velocity;

  // Apply movement with collision detection (X and Z axes independently)
  // This allows "sliding" along walls instead of getting stuck

  // Try to move on X axis
  if (!CheckCollision(
          glm::vec3(currentPos.x + proposedMove.x, currentPos.y, currentPos.z),
          currentMaze)) {
    camera->Position.x += proposedMove.x;
  }

  // Try to move on Z axis
  if (!CheckCollision(glm::vec3(camera->Position.x, currentPos.y,
                                currentPos.z + proposedMove.z),
                      currentMaze)) {
    camera->Position.z += proposedMove.z;
  }

  // Keep player at constant height (prevent floating or sinking)
  camera->Position.y = 0.5f;
}

/**
 * Process Mouse Movement
 * Updates camera orientation based on mouse movement (look around)
 * @param xoffset Horizontal mouse movement
 * @param yoffset Vertical mouse movement
 * @param constrainPitch Prevent camera from flipping upside down
 */
void Game::ProcessMouseMovement(float xoffset, float yoffset,
                                bool constrainPitch) {
  // Ignore mouse input when game is paused
  if (isPaused) {
    return;
  }
  // Delegate to camera's built-in mouse movement handler
  camera->ProcessMouseMovement(xoffset, yoffset, constrainPitch);
}

// ============================================================================
// GAME UPDATE (Network & Game Logic)
// ============================================================================

/**
 * Update Game State
 * Handles network communication (accept connections, receive unlock messages)
 * and checks portal proximity for win conditions
 * @param dt Delta time since last frame
 */
void Game::Update(float dt) {
  // ---------------------------------------------------------------------------
  // HOST MODE: Accept incoming client connections (non-blocking)
  // ---------------------------------------------------------------------------
  if (mode == GameMode::HOST && serverSocket >= 0 && clientSocket < 0) {
    // Non-blocking accept check
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(serverSocket, &readfds);
    struct timeval tv = {0, 0}; // No wait

    if (select(serverSocket + 1, &readfds, NULL, NULL, &tv) > 0) {
      clientSocket = Network::acceptConnection(serverSocket);
      if (clientSocket >= 0) {
        std::cout << "✓ Client connected!" << std::endl;
      }
    }
  }

  // CLIENT MODE: Check for unlock message
  if (mode == GameMode::CLIENT && movementLocked && networkSocket >= 0) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(networkSocket, &readfds);
    struct timeval tv = {0, 0}; // Non-blocking

    if (select(networkSocket + 1, &readfds, NULL, NULL, &tv) > 0) {
      char buffer[256];
      ssize_t bytesRead =
          Network::receiveData(networkSocket, buffer, sizeof(buffer) - 1);
      if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::string message(buffer);

        if (message.find("UNLOCK") != std::string::npos) {
          movementLocked = false;

          // Parse color tint from message (format: "UNLOCK r g b")
          float r, g, b;
          if (sscanf(buffer, "UNLOCK %f %f %f", &r, &g, &b) == 3) {
            inheritedColorTint = glm::vec3(r, g, b);
            std::cout << "\n*** UNLOCKED with inherited color tint! ***"
                      << std::endl;
          } else {
            std::cout << "\n*** UNLOCKED! You can now move! ***" << std::endl;
          }
          std::cout << "Navigate to the portal to win!\n" << std::endl;
        }
      }
    }
  }

  // Check if player is near portal
  CheckPortalProximity();
}

void Game::Render() {
  gameShader->use();

  // Configure flashlight (follows camera)
  gameShader->setVec3("light.position", camera->Position.x, camera->Position.y,
                      camera->Position.z);
  gameShader->setVec3("light.direction", camera->Front.x, camera->Front.y,
                      camera->Front.z);
  gameShader->setVec3("viewPos", camera->Position.x, camera->Position.y,
                      camera->Position.z);

  // Configure spotlight cone angles (cosine of angle)
  gameShader->setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
  gameShader->setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));

  // Light colors
  gameShader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
  gameShader->setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
  gameShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

  // Attenuation (values for ~50 meters coverage)
  gameShader->setFloat("light.constant", 1.0f);
  gameShader->setFloat("light.linear", 0.09f);
  gameShader->setFloat("light.quadratic", 0.032f);

  // View/Projection matrices
  glm::mat4 projection = glm::perspective(
      glm::radians(camera->Zoom), (float)Width / (float)Height, 0.1f, 100.0f);
  glm::mat4 view = camera->GetViewMatrix();

  gameShader->setMat4("projection", glm::value_ptr(projection));
  gameShader->setMat4("view", glm::value_ptr(view));

  // Calculate and set environment tint based on portal proximity
  glm::vec3 envTint = GetEnvironmentTint();
  gameShader->setVec3("environmentTint", envTint.x, envTint.y, envTint.z);

  // Render outdoor ground first (underneath everything)
  if (outdoorGroundMesh) {
    glm::mat4 groundModel = glm::mat4(1.0f);
    gameShader->setMat4("model", glm::value_ptr(groundModel));
    gameShader->setVec3("objectColor", 1.0f, 1.0f, 1.0f);
    gameShader->setBool("useTexture", true);
    outdoorGroundMesh->Draw(gameShader->ID);
  }

  // Render maze
  if (currentMaze) {
    currentMaze->Draw(*gameShader);
  }

  // Render trees around the perimeter
  if (treeMesh && treePositions.size() > 0) {
    gameShader->setBool("useTexture", false);

    for (const glm::vec3 &treePos : treePositions) {
      glm::mat4 treeModel = glm::mat4(1.0f);
      treeModel = glm::translate(treeModel, treePos);
      gameShader->setMat4("model", glm::value_ptr(treeModel));

      // Set tree colors (brown trunk + green canopy rendered together)
      gameShader->setVec3("objectColor", 0.3f, 0.6f,
                          0.2f); // Green-ish for overall tree
      treeMesh->Draw(gameShader->ID);
    }
  }

  // Render portal environment at maze end
  if (gateMesh && currentMaze) {
    gameShader->setBool("useTexture", false);

    // Position portal at maze end
    glm::vec3 gatePos(currentMaze->endParams.x * currentMaze->cellSize, 0.0f,
                      currentMaze->endParams.y * currentMaze->cellSize);

    glm::mat4 gateModel = glm::mat4(1.0f);
    gateModel = glm::translate(gateModel, gatePos);
    gateModel = glm::scale(
        gateModel,
        glm::vec3(1.0f, 1.0f, 1.0f)); // Portal is already sized correctly

    gameShader->setMat4("model", glm::value_ptr(gateModel));
    gameShader->setVec3("objectColor", 0.3f, 0.6f, 0.9f); // Glowing cyan portal
    gateMesh->Draw(gameShader->ID);
  }

  // Render intro dialog overlay if needed
  if (showingIntroDialog) {
    RenderIntroDialog();
  }
}

void Game::CheckPortalProximity() {
  if (!camera || connectedToPortal) {
    return; // Already connected or no camera
  }

  // Calculate distance to portal
  float distance = glm::distance(camera->Position, portalPosition);
  float proximityThreshold = 2.0f; // Trigger when within 2 units

  if (distance < proximityThreshold) {
    if (mode == GameMode::HOST) {
      // HOST MODE: Send unlock message to client
      std::cout << "\n=== PORTAL REACHED ===" << std::endl;
      std::cout << "You have reached the portal!" << std::endl;

      // Pause game and show cursor
      isPaused = true;
      if (windowPtr) {
        glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }

      std::cout << "Game PAUSED at portal." << std::endl;
      std::cout << "Press ESC to resume and continue exploring." << std::endl;

      // Send unlock message to client with current color tint
      if (clientSocket >= 0) {
        glm::vec3 currentTint = GetEnvironmentTint();
        char unlockMsg[128];
        snprintf(unlockMsg, sizeof(unlockMsg), "UNLOCK %.3f %.3f %.3f",
                 currentTint.x, currentTint.y, currentTint.z);
        Network::sendData(clientSocket, unlockMsg, strlen(unlockMsg));
        std::cout << "Sent UNLOCK signal with color tint to client!"
                  << std::endl;
      } else {
        std::cout << "No client connected to unlock." << std::endl;
      }

      connectedToPortal = true;
    } else {
      // CLIENT MODE: Victory message
      std::cout << "\n=== YOU WIN! ===" << std::endl;
      std::cout << "Congratulations! You reached the portal!" << std::endl;

      // Pause game
      isPaused = true;
      if (windowPtr) {
        glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }

      std::cout << "Press ESC to resume or close the window." << std::endl;
      connectedToPortal = true;
    }
  }
}

glm::vec3 Game::GetEnvironmentTint() {
  // Calculate distance from camera to portal
  float distance = glm::length(camera->Position - portalPosition);

  // Define maximum distance (full maze width)
  float maxDistance = currentMaze->width * currentMaze->cellSize;

  // Calculate normalized distance (0 = at portal, 1 = far from portal)
  float t = glm::clamp(1.0f - (distance / maxDistance), 0.0f, 1.0f);

  // Apply smoothstep for smooth interpolation
  t = t * t * (3.0f - 2.0f * t);

  // Define colors
  glm::vec3 normalColor(1.0f, 1.0f, 1.0f); // White/neutral
  glm::vec3 portalColor(0.4f, 0.2f,
                        1.0f); // Intense purple/mystical (more dramatic)

  // For client mode, start from inherited color instead of white
  if (mode == GameMode::CLIENT) {
    normalColor = inheritedColorTint;
  }

  // Interpolate between colors
  return glm::mix(normalColor, portalColor, t);
}

void Game::RenderIntroDialog() {
  if (!textRenderer) {
    return; // Safety check
  }

  // Initialize overlay resources once
  if (!overlayResourcesInitialized) {
    InitializeOverlayResources();
  }

  // Save current OpenGL state
  GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
  GLboolean blendEnabled = glIsEnabled(GL_BLEND);

  // Disable depth test and enable blending for 2D overlay
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Use shader and render the overlay quad
  glUseProgram(overlayShaderProgram);
  glBindVertexArray(overlayVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // Now render text on top of the overlay
  float centerX = Width / 2.0f;

  // Title
  textRenderer->RenderText("Maze: Escape from yourself", centerX - 120.0f,
                           Height - 100.0f, 1.5f, glm::vec3(1.0f, 1.0f, 1.0f));

  // Mode
  std::string modeText =
      (mode == GameMode::HOST) ? "MODE: HOST" : "MODE: CLIENT";
  textRenderer->RenderText(modeText, centerX - 100.0f, Height - 150.0f, 1.0f,
                           glm::vec3(0.8f, 0.8f, 1.0f));

  // Objective
  std::string objectiveText;
  if (mode == GameMode::HOST) {
    objectiveText = " 'No man ever steps in the same river twice, for it's not "
                    "the same river and he's not the same man.' - Heraclitus";
  } else {
    objectiveText = "Do you still feel like you are the same man? Is this "
                    "still the same river?";
  }
  textRenderer->RenderText(objectiveText, centerX - 220.0f, Height - 200.0f,
                           0.8f, glm::vec3(0.9f, 0.9f, 0.9f));

  // Controls
  textRenderer->RenderText("CONTROLS:", centerX - 100.0f, Height - 280.0f, 1.0f,
                           glm::vec3(1.0f, 0.9f, 0.5f));
  textRenderer->RenderText("WASD / Arrow Keys - Move", centerX - 160.0f,
                           Height - 320.0f, 0.7f, glm::vec3(0.9f, 0.9f, 0.9f));
  textRenderer->RenderText("Mouse - Look Around", centerX - 140.0f,
                           Height - 350.0f, 0.7f, glm::vec3(0.9f, 0.9f, 0.9f));
  textRenderer->RenderText("ESC - Pause/Resume", centerX - 130.0f,
                           Height - 380.0f, 0.7f, glm::vec3(0.9f, 0.9f, 0.9f));

  // Instruction to start
  textRenderer->RenderText("Press ENTER to Start", centerX - 150.0f, 100.0f,
                           1.0f, glm::vec3(0.5f, 1.0f, 0.5f));

  // Restore previous OpenGL state
  if (depthTestEnabled)
    glEnable(GL_DEPTH_TEST);
  if (!blendEnabled)
    glDisable(GL_BLEND);
}

void Game::InitializeOverlayResources() {
  // Simple 2D quad vertices (fullscreen overlay in NDC)
  float overlayVertices[] = {// positions (NDC: -1 to 1)
                             -1.0f, -1.0f, 0.0f, 1.0f,  -1.0f, 0.0f,
                             1.0f,  1.0f,  0.0f, 1.0f,  1.0f,  0.0f,
                             -1.0f, 1.0f,  0.0f, -1.0f, -1.0f, 0.0f};

  // Create VAO, VBO for the overlay (these will be reused)
  glGenVertexArrays(1, &overlayVAO);
  glGenBuffers(1, &overlayVBO);

  glBindVertexArray(overlayVAO);
  glBindBuffer(GL_ARRAY_BUFFER, overlayVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(overlayVertices), overlayVertices,
               GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Unbind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Create simple shader for 2D rendering
  // Vertex shader - pass through NDC coordinates
  const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    void main() {
      gl_Position = vec4(aPos, 1.0);
    }
  )";

  // Fragment shader - semi-transparent dark overlay
  const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
      FragColor = vec4(0.0, 0.0, 0.0, 0.8); // Dark semi-transparent
    }
  )";

  // Compile vertex shader
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  // Check for shader compile errors
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cerr << "ERROR: Overlay Vertex Shader Compilation Failed\n"
              << infoLog << std::endl;
  }

  // Compile fragment shader
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cerr << "ERROR: Overlay Fragment Shader Compilation Failed\n"
              << infoLog << std::endl;
  }

  // Link shaders into program
  overlayShaderProgram = glCreateProgram();
  glAttachShader(overlayShaderProgram, vertexShader);
  glAttachShader(overlayShaderProgram, fragmentShader);
  glLinkProgram(overlayShaderProgram);

  glGetProgramiv(overlayShaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(overlayShaderProgram, 512, NULL, infoLog);
    std::cerr << "ERROR: Overlay Shader Program Linking Failed\n"
              << infoLog << std::endl;
  }

  // Clean up shaders (they're linked into the program now)
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  overlayResourcesInitialized = true;
  std::cout << "Overlay resources initialized successfully!" << std::endl;
}

void Game::CleanupOverlayResources() {
  if (overlayResourcesInitialized) {
    if (overlayVAO != 0) {
      glDeleteVertexArrays(1, &overlayVAO);
      overlayVAO = 0;
    }
    if (overlayVBO != 0) {
      glDeleteBuffers(1, &overlayVBO);
      overlayVBO = 0;
    }
    if (overlayShaderProgram != 0) {
      glDeleteProgram(overlayShaderProgram);
      overlayShaderProgram = 0;
    }
    overlayResourcesInitialized = false;
  }
}
