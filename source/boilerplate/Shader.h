#pragma once

#include <glad/glad.h>

#include <string>

#include "GLHandles.h"

class ShaderProgram;
class ComputeProgram;

class Shader {
  public:
    Shader(const std::string &path, GLenum type);

    std::string getPath() const { return path; }
    GLenum getType() const { return type; }

    void friend attach(ShaderProgram &sp, Shader &s);
    void friend attach(ComputeProgram &sp, Shader &s);

  private:
    ShaderHandle shaderID;
    GLenum type;

    std::string path;

    bool compile();
};
