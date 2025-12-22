#include "Mesh.hpp"
#include "Objloader.hpp"
#include "Shader.h"
#include "kruksal/kruksal.h"
#include <vector>

class Maze {
public:
  // logic data for collision  (0 for wall, 1 for path)
  std::vector<std::vector<uint32_t>> grid;
  int width, height;
  float cellSize = 1.0f; // Tamanho de cada bloco no mundo 3D
  glm::ivec2 endParams;  // end of the maze

  // to load visual resources
  Mesh *wallMesh;  // renamed from wallModel
  Mesh *floorMesh; // renamed from floorModel
  // Texture* wallTexture  later

  // constructor
  Maze(Mesh *wMesh, Mesh *fMesh) : wallMesh(wMesh), floorMesh(fMesh) {}

  // Função que chama o teu algoritmo
  void Generate(int w, int h);

  // Função para desenhar no Loop
  void Draw(Shader &shader);

  // Função para verificar colisão
  bool IsWall(float x, float z);
};