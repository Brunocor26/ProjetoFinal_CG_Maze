#include "../include/Maze.h"
#include <glm/gtc/type_ptr.hpp>

void Maze::Generate(int w, int h) {
  // Algorithm only works with odd numbers for maze size
  if (w % 2 == 0)
    w++;
  if (h % 2 == 0)
    h++;

  this->width = w;
  this->height = h;

  // use our imported code
  try {
    // initializes the generator
    maze::kruskal generator(h, w);

    // generate the maze
    generator.generate();

    // extract the 2d grid
    this->grid = generator.get_maze();

    this->grid = generator.get_maze();

    // Find end position (last path cell)
    for (int z = height - 1; z >= 0; z--) {
      for (int x = width - 1; x >= 0; x--) {
        if (this->grid[z][x] == 1) {
          this->endParams = glm::ivec2(x, z);
          goto found_end;
        }
      }
    }
  found_end:

    std::cout << "Labirinto gerado com sucesso: " << w << "x" << h << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Erro ao gerar labirinto: " << e.what() << std::endl;
  }
}

void Maze::Draw(Shader &shader) {
  // Iterar pela grelha
  for (int z = 0; z < height; z++) {
    for (int x = 0; x < width; x++) {

      // Calcular posição no mundo
      // X na grelha -> X no mundo
      // Z na grelha -> Z no mundo
      // Y no mundo é 0
      glm::vec3 position(x * cellSize, 0.0f, z * cellSize);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, position);
      shader.setMat4("model", glm::value_ptr(model));

      // VERIFICAÇÃO: O código do Ferenc usa 0 para parede e 1 para caminho

      shader.setBool("useTexture", true); // Enable texturing

      if (grid[z][x] == 0) {
        // É UMA PAREDE
        shader.setVec3("objectColor", 1.0f, 1.0f,
                       1.0f); // White to not tint texture
        // Texture binding is handled by Mesh::Draw
        wallMesh->Draw(shader.ID);
      } else if (x == endParams.x && z == endParams.y) {
        // END POINT - Green
        shader.setVec3("objectColor", 0.0f, 1.0f, 0.0f);
        // Texture binding is handled by Mesh::Draw
        floorMesh->Draw(shader.ID);
      } else {
        // É UM CAMINHO (Desenha o chão)
        shader.setVec3("objectColor", 0.6f, 0.6f,
                       0.6f); // Grey to see texture better
        // Texture binding is handled by Mesh::Draw
        floorMesh->Draw(shader.ID);
      }
    }
  }
}

bool Maze::IsWall(float worldX, float worldZ) {
  // 1. Converter coordenadas do Mundo -> Índices da Grelha
  // Adicionamos 0.5f para arredondar para o centro da célula mais próxima
  int gridX = (int)((worldX / cellSize) + 0.5f);
  int gridZ = (int)((worldZ / cellSize) + 0.5f);

  // 2. Verificar limites (para não crashar o jogo fora do mapa)
  if (gridX < 0 || gridX >= width || gridZ < 0 || gridZ >= height) {
    return true; // Considerar fora do mapa como parede
  }

  // 3. Verificar o valor na grelha
  // Lembra-te: O algoritmo retorna 0 para parede, 1 para caminho
  return (grid[gridZ][gridX] == 0);
}