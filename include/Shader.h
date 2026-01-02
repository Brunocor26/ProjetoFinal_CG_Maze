/**
 * @file Shader.h
 * @brief Class for OpenGL shader management
 * @author Project CG - Maze Game
 * @date 2025
 */

#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

/**
 * @brief Class creating and managing OpenGL shaders
 *
 * Facilitates loading, compiling, and using shaders.
 * Supports vertex and fragment shaders, with error checking.
 *
 * Provides methods to set uniforms of various types
 * (bool, int, float, vec2, vec3, mat4).
 *
 * @note The destructor does not free the shader program. For more
 * complex applications, consider adding glDeleteProgram in the destructor.
 */
class Shader {
public:
  /// OpenGL shader program ID
  unsigned int ID;

  /**
   * @brief Constructor - loads and compiles shaders
   *
   * Reads the vertex and fragment shader files, compiles them,
   * and links them to create the shader program.
   *
   * @param vertexPath Path to the vertex shader file
   * @param fragmentPath Path to the fragment shader file
   *
   * @throws std::ifstream::failure if unable to read the files
   *
   * @note Compilation/linking errors are printed to cout
   */
  Shader(const char *vertexPath, const char *fragmentPath) {
    // 1. Read shader code
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
      std::cout << "ERROR: Shader file not read successfully" << std::endl;
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // 2. Compile shaders
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

    // Shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // Delete shaders (already linked)
    glDeleteShader(vertex);
    glDeleteShader(fragment);
  }

  /**
   * @brief Activates this shader for rendering
   *
   * Calls glUseProgram with this shader's ID.
   * All subsequent rendering calls will use this shader.
   */
  void use() { glUseProgram(ID); }

  /**
   * @brief Sets boolean uniform
   * @param name Name of the uniform variable in the shader
   * @param value Boolean value to set
   */
  void setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }

  /**
   * @brief Sets integer uniform
   * @param name Name of the uniform variable in the shader
   * @param value Integer value to set
   */
  void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }

  /**
   * @brief Sets float uniform
   * @param name Name of the uniform variable in the shader
   * @param value Float value to set
   */
  void setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }

  /**
   * @brief Sets vec2 uniform
   * @param name Name of the uniform variable in the shader
   * @param x X component of the vector
   * @param y Y component of the vector
   */
  void setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
  }

  /**
   * @brief Sets vec3 uniform
   * @param name Name of the uniform variable in the shader
   * @param x X component of the vector
   * @param y Y component of the vector
   * @param z Z component of the vector
   */
  void setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
  }

  /**
   * @brief Sets mat4 uniform (4x4 matrix)
   *
   * Used for transformations (model, view, projection).
   *
   * @param name Name of the uniform variable in the shader
   * @param value Pointer to array of 16 floats (column-major matrix)
   */
  void setMat4(const std::string &name, const float *value) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       value);
  }

private:
  /**
   * @brief Checks for compilation/linking errors
   *
   * Prints detailed error messages if compilation
   * or linking fails.
   *
   * @param shader ID of the shader or program to check
   * @param type Type of check ("VERTEX", "FRAGMENT" or "PROGRAM")
   */
  void checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "SHADER COMPILATION ERROR type: " << type << "\n"
                  << infoLog << std::endl;
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "PROGRAM LINKING ERROR\n" << infoLog << std::endl;
      }
    }
  }
};

#endif // SHADER_H
