#pragma once
#include <glad/glad.h>

#include <vector>

#include <glm/glm.hpp>

#include "VertexArray.h"
#include "GLBuffers.h"

class Vert_Geometry {
  public:
    Vert_Geometry();
    // Public interface
    void bind() const;
    void setVerts(const std::vector<glm::vec3> &verts);

  private:
    VertexArray vao;
    VertexBuffer vertBuffer;
};

class Coord_Geometry {
  public:
    Coord_Geometry();
    // Public interface
    void bind() const;
    void setCoords(const std::vector<glm::ivec3> &coords);

  private:
    VertexArray vao;
    VertexBuffer coordBuffer;
};
