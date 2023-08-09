#pragma once
#include <glad/glad.h>

#include <memory>

#include "GLHandles.h"

class VertexBuffer {
  public:
    VertexBuffer(GLuint index, GLint size, GLenum dataType);
    void bind() const;
    void uploadData(GLsizeiptr size, const void *data, GLenum usage);

  private:
    BufferHandle bufferID;
};

class IndexBuffer {
  public:
    IndexBuffer(GLuint index, GLint size, GLenum dataType);
    void bind() const;
    void uploadData(GLsizeiptr size, const void *data, GLenum usage);

  private:
    BufferHandle bufferID;
};

class ShaderStorageBuffer {
  public:
    ShaderStorageBuffer();

    void bind() const;
    void unbind() const;
    void bindBase(GLuint index) const;
    void uploadData(GLsizeiptr size, const void *data, GLenum usage);
    void updateData(GLsizeiptr size, const void *data, GLuint offset);

    void *getPointer();
    void disablePointer();

    GLint currentByteSize();

    static std::shared_ptr<ShaderStorageBuffer> clone(std::shared_ptr<ShaderStorageBuffer> src);

  private:
    BufferHandle bufferID;
    GLint current_byte_size;
};