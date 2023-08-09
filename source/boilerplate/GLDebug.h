#pragma once
#include <glad/glad.h>

namespace GLDebug {
void debugOutputHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar *message, const void *);

void enable();
}  // namespace GLDebug
