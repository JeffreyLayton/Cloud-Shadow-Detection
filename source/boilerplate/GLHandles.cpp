#include <algorithm>

#include "GLHandles.h"

ShaderHandle::ShaderHandle(GLenum type)
    : shaderID(glCreateShader(type)) {}

ShaderHandle::ShaderHandle(ShaderHandle &&other) noexcept
    : shaderID(std::move(other.shaderID)) {
    other.shaderID = 0;
}

ShaderHandle &ShaderHandle::operator=(ShaderHandle &&other) noexcept {
    std::swap(shaderID, other.shaderID);
    return *this;
}

ShaderHandle::~ShaderHandle() { glDeleteShader(shaderID); }

ShaderHandle::operator GLuint() const { return shaderID; }

GLuint ShaderHandle::value() const { return shaderID; }

//------------------------------------------------------------------------------

ShaderProgramHandle::ShaderProgramHandle()
    : programID(glCreateProgram()) {}

ShaderProgramHandle::ShaderProgramHandle(ShaderProgramHandle &&other) noexcept
    : programID(std::move(other.programID)) {
    other.programID = 0;
}

ShaderProgramHandle &ShaderProgramHandle::operator=(ShaderProgramHandle &&other) noexcept {
    std::swap(programID, other.programID);
    return *this;
}

ShaderProgramHandle::~ShaderProgramHandle() { glDeleteProgram(programID); }

ShaderProgramHandle::operator GLuint() const { return programID; }

GLuint ShaderProgramHandle::value() const { return programID; }

//------------------------------------------------------------------------------

VertexArrayHandle::VertexArrayHandle()
    : vaoID(0) {
    glGenVertexArrays(1, &vaoID);
}

VertexArrayHandle::VertexArrayHandle(VertexArrayHandle &&other) noexcept
    : vaoID(std::move(other.vaoID)) {
    other.vaoID = 0;
}

VertexArrayHandle &VertexArrayHandle::operator=(VertexArrayHandle &&other) noexcept {
    std::swap(vaoID, other.vaoID);
    return *this;
}

VertexArrayHandle::~VertexArrayHandle() { glDeleteVertexArrays(1, &vaoID); }

VertexArrayHandle::operator GLuint() const { return vaoID; }

GLuint VertexArrayHandle::value() const { return vaoID; }

//------------------------------------------------------------------------------

BufferHandle::BufferHandle()
    : vboID(0) {
    glGenBuffers(1, &vboID);
}

BufferHandle::BufferHandle(BufferHandle &&other) noexcept
    : vboID(std::move(other.vboID)) {
    other.vboID = 0;
}

BufferHandle &BufferHandle::operator=(BufferHandle &&other) noexcept {
    std::swap(vboID, other.vboID);
    return *this;
}

BufferHandle::~BufferHandle() { glDeleteBuffers(1, &vboID); }

BufferHandle::operator GLuint() const { return vboID; }

GLuint BufferHandle::value() const { return vboID; }