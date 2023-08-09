#pragma once

#include <glad/glad.h>

#include "GLHandles.h"

class VertexArray {
  public:
    VertexArray();
    void bind() const;

  private:
    VertexArrayHandle arrayID;
};
