#include "Geometry.h"

Vert_Geometry::Vert_Geometry()
    : vao()
    , vertBuffer(0, 3, GL_FLOAT) {}

void Vert_Geometry::bind() const { vao.bind(); }

void Vert_Geometry::setVerts(const std::vector<glm::vec3> &verts) {
    vertBuffer.uploadData(sizeof(glm::vec3) * verts.size(), verts.data(), GL_STATIC_DRAW);
}

Coord_Geometry::Coord_Geometry()
    : vao()
    , coordBuffer(0, 3, GL_INT) {}

void Coord_Geometry::bind() const { vao.bind(); }

void Coord_Geometry::setCoords(const std::vector<glm::ivec3> &coords) {
    coordBuffer.uploadData(sizeof(glm::ivec3) * coords.size(), coords.data(), GL_STATIC_DRAW);
}