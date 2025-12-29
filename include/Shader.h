/**
 * @file Shader.h
 * @brief Classe para gestão de shaders OpenGL
 * @author Projeto CG - Maze Game
 * @date 2025
 */

#ifndef SHADER_H
#define SHADER_H

#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <string>

/**
 * @brief Classe para criar e gerir shaders OpenGL
 *
 * Facilita o carregamento, compilação e uso de shaders.
 * Suporta vertex e fragment shaders, com verificação de erros.
 *
 * Fornece métodos para definir uniforms de vários tipos
 * (bool, int, float, vec2, vec3, mat4).
 *
 * @note O destrutor não liberta o programa shader. Para aplicações
 * mais complexas, considerar adicionar glDeleteProgram no destrutor.
 */
class Shader {
public:
  /// ID do programa shader OpenGL
  unsigned int ID;

  /**
   * @brief Construtor - carrega e compila shaders
   *
   * Lê os ficheiros de vertex e fragment shader, compila-os
   * e faz link para criar o programa shader.
   *
   * @param vertexPath Caminho para o ficheiro do vertex shader
   * @param fragmentPath Caminho para o ficheiro do fragment shader
   *
   * @throws std::ifstream::failure se não conseguir ler os ficheiros
   *
   * @note Erros de compilação/linking são impressos no cout
   */
  Shader(const char *vertexPath, const char *fragmentPath) {
    // 1. Ler código dos shaders
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
      vShaderFile.open(vertexPath);
      fShaderFile.open(fragmentPath);
      std::stringstream vShaderStream, fShaderStream;

      vShaderStream << vShaderFile.rdbuf();
      fShaderStream << fShaderFile.rdbuf();

      vShaderFile.close();
      fShaderFile.close();

      vertexCode = vShaderStream.str();
      fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure &e) {
      std::cout << "ERRO: Shader file não foi lido corretamente" << std::endl;
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // 2. Compilar shaders
    unsigned int vertex, fragment;

    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // Fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // Programa de shader
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // Eliminar shaders (já linkados)
    glDeleteShader(vertex);
    glDeleteShader(fragment);
  }

  /**
   * @brief Ativa este shader para renderização
   *
   * Chama glUseProgram com o ID deste shader.
   * Todas as chamadas de renderização subsequentes usarão este shader.
   */
  void use() { glUseProgram(ID); }

  /**
   * @brief Define uniform booleano
   * @param name Nome da variável uniform no shader
   * @param value Valor booleano a definir
   */
  void setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }

  /**
   * @brief Define uniform inteiro
   * @param name Nome da variável uniform no shader
   * @param value Valor inteiro a definir
   */
  void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }

  /**
   * @brief Define uniform float
   * @param name Nome da variável uniform no shader
   * @param value Valor float a definir
   */
  void setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }

  /**
   * @brief Define uniform vec2
   * @param name Nome da variável uniform no shader
   * @param x Componente X do vetor
   * @param y Componente Y do vetor
   */
  void setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
  }

  /**
   * @brief Define uniform vec3
   * @param name Nome da variável uniform no shader
   * @param x Componente X do vetor
   * @param y Componente Y do vetor
   * @param z Componente Z do vetor
   */
  void setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
  }

  /**
   * @brief Define uniform mat4 (matriz 4x4)
   *
   * Usado para transformações (model, view, projection).
   *
   * @param name Nome da variável uniform no shader
   * @param value Ponteiro para array de 16 floats (matriz em column-major)
   */
  void setMat4(const std::string &name, const float *value) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       value);
  }

private:
  /**
   * @brief Verifica erros de compilação/linking
   *
   * Imprime mensagens de erro detalhadas se a compilação
   * ou linking falharem.
   *
   * @param shader ID do shader ou programa a verificar
   * @param type Tipo de verificação ("VERTEX", "FRAGMENT" ou "PROGRAM")
   */
  void checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERRO COMPILAÇÃO SHADER do tipo: " << type << "\n"
                  << infoLog << std::endl;
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERRO LINKING PROGRAMA\n" << infoLog << std::endl;
      }
    }
  }
};

#endif // SHADER_H
