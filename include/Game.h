#include "../include/learnopengl/camera.h"
#include "Maze.h"

// Forward declaration
struct GLFWwindow;

enum class GameMode { HOST, CLIENT };

class Game {
public:
  // game state
  bool Keys[1024];
  unsigned int Width, Height;

  // Game mode
  GameMode mode;
  bool movementLocked; // For client mode
  int serverSocket;    // For host mode (listening socket)
  int clientSocket;    // For host mode (connected client)

  // constructor and destructor
  Game(unsigned int width, unsigned int height,
       GameMode gameMode = GameMode::HOST);
  ~Game();

  // Ciclo de vida
  void Init();                 // loads shaders, models, textures
  void ProcessInput(float dt); // process wasd and keyboard inputs
  void ProcessMouseMovement(float xoffset, float yoffset,
                            bool constrainPitch = true); // process mouse input
  void Update(float dt); // logics (ex: move enemies, open doors)
  void Render();         // draws everything (calls Draws from Meshes)

  // Membros
  Maze *currentMaze; // Ponteiro para o nível atual
  Camera *camera;    // player camera

  // Outdoor environment
  Mesh *outdoorGroundMesh;              // Large ground plane with grass
  Mesh *treeMesh;                       // Tree model
  std::vector<glm::vec3> treePositions; // Positions for tree instances

  // Gate at maze end
  Mesh *gateMesh; // Gate model for maze exit

  // Network connection
  int networkSocket;
  bool connectedToPortal;
  glm::vec3 portalPosition;

  // Helper method
  void CheckPortalProximity();

  // Pause state
  bool isPaused;
  GLFWwindow *windowPtr; // Store window pointer for cursor control

  // Intro dialog state
  bool showingIntroDialog;
  void RenderIntroDialog();
  class TextRenderer *textRenderer; // Text rendering system
};