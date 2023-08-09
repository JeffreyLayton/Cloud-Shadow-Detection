#pragma once

#include <glad/glad.h>

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLHandles.h"
#include "Shader.h"

class ShaderProgram {
  public:
    ShaderProgram(
        const std::string &vertexPath,
        const std::string &geometryPath,
        const std::string &fragmentPath
    );
    ShaderProgram(const std::string &vertexPath, const std::string &fragmentPath);

    // Public interface
    bool recompile();
    void use() const { glUseProgram(programID); }

    void friend attach(ShaderProgram &sp, Shader &s);

    operator GLuint() const { return programID; }

    GLuint getUniformLocation(std::string name);

    void setMat2(glm::mat2 value, std::string name);
    void setMat3(glm::mat3 value, std::string name);
    void setMat4(glm::mat4 value, std::string name);

    void setFloat(float value, std::string name);
    void setVec2(glm::vec2 value, std::string name);
    void setVec3(glm::vec3 value, std::string name);
    void setVec4(glm::vec4 value, std::string name);

    void setFloatv(std::vector<float> values, std::string name);
    void setVec2v(std::vector<glm::vec2> values, std::string name);
    void setVec3v(std::vector<glm::vec3> values, std::string name);
    void setVec4v(std::vector<glm::vec4> values, std::string name);

    void setInt(int value, std::string name);
    void setIvec2(glm::ivec2 value, std::string name);
    void setIvec3(glm::ivec3 value, std::string name);
    void setIvec4(glm::ivec4 value, std::string name);

    void setBoolean(bool value, std::string name);
    void setUint(unsigned int value, std::string name);

  private:
    ShaderProgramHandle programID;

    std::unique_ptr<Shader> vertexShader;
    std::unique_ptr<Shader> geometryShader;
    std::unique_ptr<Shader> fragmentShader;

    bool checkAndLogLinkSuccess() const;
};
