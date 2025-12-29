/**
 * @file Maze.h
 * @brief Declaração da classe Maze - gestão do labirinto 3D
 * @author Projeto CG - Maze Game
 * @date 2025
 */

#ifndef MAZE_H
#define MAZE_H

#include "Mesh.hpp"
#include "Objloader.hpp"
#include "Shader.h"
#include "kruksal/kruksal.h"
#include <vector>

/**
 * @brief Classe que representa e gere o labirinto 3D
 *
 * Responsável por:
 * - Geração procedural do labirinto usando algoritmo de Kruskal
 * - Renderização das paredes e chão
 * - Deteção de colisões com paredes
 * - Gestão da estrutura lógica (grid) e visual (meshes)
 *
 * O labirinto é representado por uma grid 2D onde:
 * - 0 = parede (colisão)
 * - 1 = caminho (livre para mover)
 */
class Maze {
public:
  // ========================================================================
  // DADOS LÓGICOS
  // ========================================================================

  /**
   * @brief Grid 2D do labirinto
   *
   * Matriz que representa a estrutura lógica:
   * - grid[y][x] = 0 : parede (bloqueado)
   * - grid[y][x] = 1 : caminho (livre)
   */
  std::vector<std::vector<uint32_t>> grid;

  /// Largura do labirinto (número de células)
  int width;

  /// Altura do labirinto (número de células)
  int height;

  /// Tamanho de cada célula no mundo 3D (em unidades OpenGL)
  float cellSize = 1.0f;

  /**
   * @brief Coordenadas da célula final do labirinto
   *
   * Indica onde está o portal de saída (x, y na grid).
   */
  glm::ivec2 endParams;

  // ========================================================================
  // RECURSOS VISUAIS
  // ========================================================================

  /// Mesh usada para renderizar as paredes
  Mesh *wallMesh;

  /// Mesh usada para renderizar o chão
  Mesh *floorMesh;

  // ========================================================================
  // CONSTRUTOR
  // ========================================================================

  /**
   * @brief Construtor do labirinto
   *
   * Inicializa o labirinto com as meshes fornecidas.
   * O labirinto ainda não está gerado - chamar Generate() depois.
   *
   * @param wMesh Ponteiro para a mesh das paredes
   * @param fMesh Ponteiro para a mesh do chão
   */
  Maze(Mesh *wMesh, Mesh *fMesh) : wallMesh(wMesh), floorMesh(fMesh) {}

  // ========================================================================
  // MÉTODOS PRINCIPAIS
  // ========================================================================

  /**
   * @brief Gera o labirinto proceduralmente
   *
   * Usa o algoritmo de Kruskal para criar um labirinto perfeito
   * (sem ciclos, apenas um caminho entre quaisquer dois pontos).
   *
   * Preenche a grid com 0s (paredes) e 1s (caminhos) e define
   * as posições de início e fim.
   *
   * @param w Largura desejada (número de células)
   * @param h Altura desejada (número de células)
   */
  void Generate(int w, int h);

  /**
   * @brief Renderiza o labirinto
   *
   * Desenha todas as paredes e células de chão usando as meshes
   * fornecidas. Percorre a grid e desenha cada célula conforme
   * o seu tipo (parede ou chão).
   *
   * @param shader Referência ao shader usado para renderização
   */
  void Draw(Shader &shader);

  /**
   * @brief Verifica se uma posição 3D contém uma parede
   *
   * Converte coordenadas do mundo 3D para coordenadas da grid
   * e verifica se a célula é uma parede (colisão).
   *
   * @param x Coordenada X no mundo 3D
   * @param z Coordenada Z no mundo 3D
   * @return true se houver parede (colisão), false caso contrário
   */
  bool IsWall(float x, float z);
};

#endif // MAZE_H