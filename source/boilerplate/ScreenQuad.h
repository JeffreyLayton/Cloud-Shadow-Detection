#pragma once

#include <memory>

#include "GLBuffers.h"
#include "VertexArray.h"
#include "glm/glm.hpp"

class ScreenQuad {
  public:
    ScreenQuad(bool x_flip, bool y_flip);
    void draw();

  private:
    std::shared_ptr<VertexArray> vao;
    std::shared_ptr<VertexBuffer> vertBuffer;
    std::shared_ptr<VertexBuffer> uvBuffer;
};
