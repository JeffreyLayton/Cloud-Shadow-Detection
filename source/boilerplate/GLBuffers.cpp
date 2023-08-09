#include "GLBuffers.h"

VertexBuffer::VertexBuffer(GLuint index, GLint size, GLenum dataType)
    : bufferID{} {
    bind();
    if (dataType == GL_BYTE || dataType == GL_UNSIGNED_BYTE || dataType == GL_SHORT
        || dataType == GL_UNSIGNED_SHORT || dataType == GL_INT || dataType == GL_UNSIGNED_INT)
        glVertexAttribIPointer(index, size, dataType, 0, (void *)0);
    else glVertexAttribPointer(index, size, dataType, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(index);
}

void VertexBuffer::bind() const { glBindBuffer(GL_ARRAY_BUFFER, bufferID); }

void VertexBuffer::uploadData(GLsizeiptr size, const void *data, GLenum usage) {
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

IndexBuffer::IndexBuffer(GLuint index, GLint size, GLenum dataType)
    : bufferID{} {
    bind();
    glVertexAttribPointer(index, size, dataType, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(index);
}

void IndexBuffer::bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID); }

void IndexBuffer::uploadData(GLsizeiptr size, const void *data, GLenum usage) {
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

ShaderStorageBuffer::ShaderStorageBuffer()
    : bufferID()
    , current_byte_size(0) {}

void ShaderStorageBuffer::bind() const { glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID); }
void ShaderStorageBuffer::unbind() const { glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); }

void ShaderStorageBuffer::bindBase(GLuint index) const {
    bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, bufferID);
}

void ShaderStorageBuffer::uploadData(GLsizeiptr size, const void *data, GLenum usage) {
    bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    current_byte_size = GLint(size);
}

void ShaderStorageBuffer::updateData(GLsizeiptr size, const void *data, GLuint offset) {
    bind();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

void *ShaderStorageBuffer::getPointer() {
    bind();
    return glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
}

void ShaderStorageBuffer::disablePointer() { glUnmapBuffer(GL_SHADER_STORAGE_BUFFER); }

GLint ShaderStorageBuffer::currentByteSize() { return current_byte_size; }

std::shared_ptr<ShaderStorageBuffer> ShaderStorageBuffer::clone(
    std::shared_ptr<ShaderStorageBuffer> src
) {
    std::shared_ptr<ShaderStorageBuffer> ret = std::make_shared<ShaderStorageBuffer>();
    ret->uploadData(src->currentByteSize(), nullptr, GL_DYNAMIC_DRAW);
    ret->current_byte_size = src->currentByteSize();
    glCopyNamedBufferSubData(src->bufferID, ret->bufferID, 0, 0, src->currentByteSize());
    return ret;
}