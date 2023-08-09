#pragma once

#include <glad/glad.h>

class ShaderHandle {
  public:
    ShaderHandle(GLenum type);

    ShaderHandle(const ShaderHandle &)           = delete;
    ShaderHandle operator=(const ShaderHandle &) = delete;

    ShaderHandle(ShaderHandle &&other) noexcept;
    ShaderHandle &operator=(ShaderHandle &&other) noexcept;

    ~ShaderHandle();

    operator GLuint() const;
    GLuint value() const;

  private:
    GLuint shaderID;
};

class ShaderProgramHandle {
  public:
    ShaderProgramHandle();

    ShaderProgramHandle(const ShaderProgramHandle &)           = delete;
    ShaderProgramHandle operator=(const ShaderProgramHandle &) = delete;

    ShaderProgramHandle(ShaderProgramHandle &&other) noexcept;
    ShaderProgramHandle &operator=(ShaderProgramHandle &&other) noexcept;

    ~ShaderProgramHandle();

    operator GLuint() const;
    GLuint value() const;

  private:
    GLuint programID;
};

class VertexArrayHandle {
  public:
    VertexArrayHandle();

    VertexArrayHandle(const VertexArrayHandle &)           = delete;
    VertexArrayHandle operator=(const VertexArrayHandle &) = delete;

    VertexArrayHandle(VertexArrayHandle &&other) noexcept;
    VertexArrayHandle &operator=(VertexArrayHandle &&other) noexcept;

    ~VertexArrayHandle();

    operator GLuint() const;
    GLuint value() const;

  private:
    GLuint vaoID;
};

class BufferHandle {
  public:
    BufferHandle();

    BufferHandle(const BufferHandle &)           = delete;
    BufferHandle operator=(const BufferHandle &) = delete;

    BufferHandle(BufferHandle &&other) noexcept;
    BufferHandle &operator=(BufferHandle &&other) noexcept;

    ~BufferHandle();

    operator GLuint() const;
    GLuint value() const;

  private:
    GLuint vboID;
};
