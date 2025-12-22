#include "../include/learnopengl/camera.h"
#include "Maze.h"

class Game {
public:
  // game state
  bool Keys[1024];
  unsigned int Width, Height;

  // constructor and destructor
  Game(unsigned int width, unsigned int height);
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
};