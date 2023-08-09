#include <iostream>
#include <stdexcept>
#include <vector>

#include "ShaderProgram.h"

#include "Log.h"

ShaderProgram::ShaderProgram(
    const std::string &vertexPath,
    const std::string &geometryPath,
    const std::string &fragmentPath
)
    : programID() {
    vertexShader   = std::make_unique<Shader>(vertexPath, GL_VERTEX_SHADER);
    geometryShader = std::make_unique<Shader>(geometryPath, GL_GEOMETRY_SHADER);
    fragmentShader = std::make_unique<Shader>(fragmentPath, GL_FRAGMENT_SHADER);
    attach(*this, *vertexShader);
    attach(*this, *geometryShader);
    attach(*this, *fragmentShader);
    glLinkProgram(programID);

    if (!checkAndLogLinkSuccess()) {
        glDeleteProgram(programID);
        throw std::runtime_error("Shaders did not link.");
    }
}

ShaderProgram::ShaderProgram(const std::string &vertexPath, const std::string &fragmentPath)
    : programID() {
    vertexShader   = std::make_unique<Shader>(vertexPath, GL_VERTEX_SHADER);
    fragmentShader = std::make_unique<Shader>(fragmentPath, GL_FRAGMENT_SHADER);
    attach(*this, *vertexShader);
    attach(*this, *fragmentShader);
    glLinkProgram(programID);

    if (!checkAndLogLinkSuccess()) {
        glDeleteProgram(programID);
        throw std::runtime_error("Shaders did not link.");
    }
}

bool ShaderProgram::recompile() {
    try {
        // Try to create a new program
        if (geometryShader) {
            ShaderProgram newProgram(
                vertexShader->getPath(), geometryShader->getPath(), fragmentShader->getPath()
            );
            *this = std::move(newProgram);
        } else {
            ShaderProgram newProgram(vertexShader->getPath(), fragmentShader->getPath());
            *this = std::move(newProgram);
        }

        return true;
    } catch (std::runtime_error &e) {
        Log::warn("SHADER_PROGRAM falling back to previous version of shaders");
        return false;
    }
}

void attach(ShaderProgram &sp, Shader &s) { glAttachShader(sp.programID, s.shaderID); }

bool ShaderProgram::checkAndLogLinkSuccess() const {
    GLint success;

    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetProgramInfoLog(programID, logLength, NULL, log.data());
        if (geometryShader) {
            Log::error(
                "SHADER_PROGRAM linking {} + {} + {}:\n{}",
                vertexShader->getPath(),
                geometryShader->getPath(),
                fragmentShader->getPath(),
                log.data()
            );
        } else {
            Log::error(
                "SHADER_PROGRAM linking {} + {}:\n{}",
                vertexShader->getPath(),
                fragmentShader->getPath(),
                log.data()
            );
        }
        return false;
    } else {
        if (geometryShader) {
            Log::info(
                "SHADER_PROGRAM successfully compiled and linked {} + {} + {}",
                vertexShader->getPath(),
                geometryShader->getPath(),
                fragmentShader->getPath()
            );
        } else {
            Log::info(
                "SHADER_PROGRAM successfully compiled and linked {} + {}",
                vertexShader->getPath(),
                fragmentShader->getPath()
            );
        }

        return true;
    }
}

GLuint ShaderProgram::getUniformLocation(std::string name) {
    return glGetUniformLocation(programID, name.c_str());
}

void ShaderProgram::setMat2(glm::mat2 value, std::string name) {
    glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}
void ShaderProgram::setMat3(glm::mat3 value, std::string name) {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}
void ShaderProgram::setMat4(glm::mat4 value, std::string name) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
};

void ShaderProgram::setFloat(float value, std::string name) {
    glUniform1fv(getUniformLocation(name), 1, &value);
}
void ShaderProgram::setVec2(glm::vec2 value, std::string name) {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}
void ShaderProgram::setVec3(glm::vec3 value, std::string name) {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}
void ShaderProgram::setVec4(glm::vec4 value, std::string name) {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void ShaderProgram::setFloatv(std::vector<float> values, std::string name) {
    glUniform1fv(getUniformLocation(name), values.size(), (GLfloat *)values.data());
}
void ShaderProgram::setVec2v(std::vector<glm::vec2> values, std::string name) {
    glUniform2fv(getUniformLocation(name), values.size(), (GLfloat *)values.data());
}
void ShaderProgram::setVec3v(std::vector<glm::vec3> values, std::string name) {
    glUniform3fv(getUniformLocation(name), values.size(), (GLfloat *)values.data());
}
void ShaderProgram::setVec4v(std::vector<glm::vec4> values, std::string name) {
    glUniform4fv(getUniformLocation(name), values.size(), (GLfloat *)values.data());
}

void ShaderProgram::setInt(int value, std::string name) {
    glUniform1iv(getUniformLocation(name), 1, &value);
}
void ShaderProgram::setIvec2(glm::ivec2 value, std::string name) {
    glUniform2iv(getUniformLocation(name), 1, glm::value_ptr(value));
}
void ShaderProgram::setIvec3(glm::ivec3 value, std::string name) {
    glUniform3iv(getUniformLocation(name), 1, glm::value_ptr(value));
}
void ShaderProgram::setIvec4(glm::ivec4 value, std::string name) {
    glUniform4iv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void ShaderProgram::setBoolean(bool value, std::string name) {
    glUniform1i(getUniformLocation(name), value ? 1 : 0);
}
void ShaderProgram::setUint(unsigned int value, std::string name) {
    glUniform1uiv(getUniformLocation(name), 1, &value);
}

// -------------------------------------------------------------------------------------------------------------------