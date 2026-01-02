/**
 * @file Maze.cpp
 * @brief Implementation of the Maze class
 * @author Project CG - Maze Game
 * @date 2025
 */

#include "../include/Maze.h"
#include <glm/gtc/type_ptr.hpp>

/**
 * @brief Generates the procedural maze
 * @param w Width of the maze
 * @param h Height of the maze
 */
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

    std::cout << "Maze generated successfully: " << w << "x" << h << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Error generating maze: " << e.what() << std::endl;
  }
}

/**
 * @brief Renders the maze
 * @param shader Reference to the shader
 */
void Maze::Draw(Shader &shader) {
  // Iterate through grid
  for (int z = 0; z < height; z++) {
    for (int x = 0; x < width; x++) {

      // Calculate world position
      // X in grid -> X in world
      // Z in grid -> Z in world
      // Y in world is 0
      glm::vec3 position(x * cellSize, 0.0f, z * cellSize);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, position);
      shader.setMat4("model", glm::value_ptr(model));

      // VERIFICATION: Ferenc's code uses 0 for wall and 1 for path

      shader.setBool("useTexture", true); // Enable texturing

      if (grid[z][x] == 0) {
        // IT'S A WALL
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
        // IT'S A PATH (Draw floor)
        shader.setVec3("objectColor", 0.6f, 0.6f,
                       0.6f); // Grey to see texture better
        // Texture binding is handled by Mesh::Draw
        floorMesh->Draw(shader.ID);
      }
    }
  }
}

/**
 * @brief Checks collision with walls
 * @param worldX X coordinate in the world
 * @param worldZ Z coordinate in the world
 * @return true if collision, false otherwise
 */
bool Maze::IsWall(float worldX, float worldZ) {
  // 1. Convert World coordinates -> Grid Indices
  // Add 0.5f to round to nearest cell center
  int gridX = (int)((worldX / cellSize) + 0.5f);
  int gridZ = (int)((worldZ / cellSize) + 0.5f);

  // 2. Check bounds (to not crash game outside map)
  if (gridX < 0 || gridX >= width || gridZ < 0 || gridZ >= height) {
    return true; // Consider outside map as wall
  }

  // 3. Check grid value
  // Remember: Algorithm returns 0 for wall, 1 for path
  return (grid[gridZ][gridX] == 0);
}

/**
 * @brief Finds the starting position
 * @return Start position as vector
 */
glm::vec3 Maze::FindStartPosition() {
  // Find valid start position
  for (int z = 0; z < height; z++) {
    for (int x = 0; x < width; x++) {
      if (grid[z][x] == 1) { // 1 is path
        std::cout << "Start Position Found at: " << x << ", " << z << std::endl;
        return glm::vec3(x * cellSize, 0.5f, z * cellSize);
      }
    }
  }
  std::cout << "CRITICAL: No path (1) found in maze grid!" << std::endl;
  return glm::vec3(0.0f, 0.5f, 0.0f); // Fallback position
}
