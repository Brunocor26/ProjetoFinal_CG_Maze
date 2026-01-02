/**
 * @file Maze.h
 * @brief Declaration of the Maze class - 3D maze management
 * @author Project CG - Maze Game
 * @date 2025
 */

#ifndef MAZE_H
#define MAZE_H

#include "Mesh.hpp"
#include "Shader.h"
#include "kruksal/kruksal.h"
#include <vector>

/**
 * @brief Class that represents and manages the 3D maze
 *
 * Responsible for:
 * - Procedural maze generation using Kruskal's algorithm
 * - Rendering walls and floor
 * - Wall collision detection
 * - Managing logical structure (grid) and visual structure (meshes)
 *
 * The maze is represented by a 2D grid where:
 * - 0 = wall (collision)
 * - 1 = path (free to move)
 */
class Maze {
public:
  // ========================================================================
  // LOGICAL DATA
  // ========================================================================

  /**
   * @brief 2D Maze Grid
   *
   * Matrix representing the logical structure:
   * - grid[y][x] = 0 : wall (blocked)
   * - grid[y][x] = 1 : path (free)
   */
  std::vector<std::vector<uint32_t>> grid;

  /// Maze width (number of cells)
  int width;

  /// Maze height (number of cells)
  int height;

  /// Size of each cell in the 3D world (in OpenGL units)
  float cellSize = 1.0f;

  /**
   * @brief Coordinates of the final maze cell
   *
   * Indicates where the exit portal is (x, y in the grid).
   */
  glm::ivec2 endParams;

  // ========================================================================
  // VISUAL RESOURCES
  // ========================================================================

  /// Mesh used to render walls
  Mesh *wallMesh;

  /// Mesh used to render the floor
  Mesh *floorMesh;

  // ========================================================================
  // CONSTRUCTOR
  // ========================================================================

  /**
   * @brief Maze constructor
   *
   * Initializes the maze with the provided meshes.
   * The maze is not generated yet - call Generate() afterwards.
   *
   * @param wMesh Pointer to wall mesh
   * @param fMesh Pointer to floor mesh
   */
  Maze(Mesh *wMesh, Mesh *fMesh) : wallMesh(wMesh), floorMesh(fMesh) {}

  // ========================================================================
  // MAIN METHODS
  // ========================================================================

  /**
   * @brief Generates the maze procedurally
   *
   * Uses Kruskal's algorithm to create a perfect maze
   * (no cycles, only one path between any two points).
   *
   * Fills the grid with 0s (walls) and 1s (paths) and sets
   * start and end positions.
   *
   * @param w Desired width (number of cells)
   * @param h Desired height (number of cells)
   */
  void Generate(int w, int h);

  /**
   * @brief Renders the maze
   *
   * Draws all walls and floor cells using the provided
   * meshes. Iterates through the grid and draws each cell according
   * to its type (wall or floor).
   *
   * @param shader Reference to the shader used for rendering
   */
  void Draw(Shader &shader);

  /**
   * @brief Checks if a 3D position contains a wall
   *
   * Converts 3D world coordinates to grid coordinates
   * and checks if the cell is a wall (collision).
   *
   * @param x X coordinate in 3D world
   * @param z Z coordinate in 3D world
   * @return true if there is a wall (collision), false otherwise
   */
  bool IsWall(float x, float z);

  /**
   * @brief Finds the start position in the maze
   *
   * Searches for the first path cell (value 1) in the grid
   * and returns its coordinates in the 3D world.
   *
   * @return glm::vec3 with the start point coordinates
   */
  glm::vec3 FindStartPosition();
};

#endif // MAZE_H