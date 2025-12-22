#include "../include/Game.h"
#include "../include/Objloader.hpp"
#include "../include/Shader.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "../include/Texture.h"
#include "../include/stb_image.h"
#include <iostream>

Shader *gameShader;
Mesh *wall_mesh;
Mesh *floor_mesh;

unsigned int loadTexture(char const *path);

Game::Game(unsigned int width, unsigned int height)
    : Width(width), Height(height), currentMaze(nullptr), camera(nullptr),
      outdoorGroundMesh(nullptr), treeMesh(nullptr), gateMesh(nullptr) {
  for (int i = 0; i < 1024; i++)
    Keys[i] = false;
}

Game::~Game() {
  delete currentMaze;
  delete camera;
  delete gameShader;
  delete wall_mesh;
  delete floor_mesh;
  delete outdoorGroundMesh;
  delete treeMesh;
  delete gateMesh;
}

void Game::Init() {
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
}

unsigned int loadTexture(char const *path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  stbi_set_flip_vertically_on_load(true);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data) {
    std::cout << "Texture Loaded! Path: " << path << " | W: " << width
              << " H: " << height << " Ch: " << nrComponents << std::endl;
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

// Helper to check collision without modifying actual position
bool CheckCollision(glm::vec3 targetPos, Maze *maze) {
  if (!maze)
    return false;
  // Simple point collision (can improve to bounding box later)
  return maze->IsWall(targetPos.x, targetPos.z);
}

void Game::ProcessInput(float dt) {
  // Current Position
  glm::vec3 currentPos = camera->Position;
  glm::vec3 nextPos = currentPos;
  float velocity = camera->MovementSpeed * dt;

  // Determine intended movement direction (X and Z only)
  glm::vec3 front = camera->Front;
  front.y = 0.0f; // Lock Y movement
  front = glm::normalize(front);
  glm::vec3 right = camera->Right;
  right.y = 0.0f; // Lock Y movement
  right = glm::normalize(right);

  glm::vec3 proposedMove = glm::vec3(0.0f);

  if (Keys[GLFW_KEY_W] || Keys[GLFW_KEY_UP])
    proposedMove += front * velocity;
  if (Keys[GLFW_KEY_S] || Keys[GLFW_KEY_DOWN])
    proposedMove -= front * velocity;
  if (Keys[GLFW_KEY_A] || Keys[GLFW_KEY_LEFT])
    proposedMove -= right * velocity;
  if (Keys[GLFW_KEY_D] || Keys[GLFW_KEY_RIGHT])
    proposedMove += right * velocity;

  // Apply X movement
  if (!CheckCollision(
          glm::vec3(currentPos.x + proposedMove.x, currentPos.y, currentPos.z),
          currentMaze)) {
    camera->Position.x += proposedMove.x;
  }

  // Apply Z movement
  if (!CheckCollision(glm::vec3(camera->Position.x, currentPos.y,
                                currentPos.z + proposedMove.z),
                      currentMaze)) {
    camera->Position.z += proposedMove.z;
  }

  // Lock Y Height
  camera->Position.y = 0.5f;
}

void Game::ProcessMouseMovement(float xoffset, float yoffset,
                                bool constrainPitch) {
  camera->ProcessMouseMovement(xoffset, yoffset, constrainPitch);
}

void Game::Update(float dt) {
  // Logic updates here
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
}
