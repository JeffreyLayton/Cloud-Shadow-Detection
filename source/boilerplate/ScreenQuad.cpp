#include <vector>

#include "ScreenQuad.h"

ScreenQuad::ScreenQuad(bool x_flip, bool y_flip) {
    vao        = std::make_shared<VertexArray>();
    vertBuffer = std::make_shared<VertexBuffer>(0, 3, GL_FLOAT);
    uvBuffer   = std::make_shared<VertexBuffer>(1, 2, GL_FLOAT);
    std::vector<glm::vec3> quad
        = {glm::vec3(1.f, 1.f, 0.f),
           glm::vec3(-1.f, 1.f, 0.f),
           glm::vec3(-1.f, -1.f, 0.f),
           glm::vec3(1.f, 1.f, 0.f),
           glm::vec3(-1.f, -1.f, 0.f),
           glm::vec3(1.f, -1.f, 0.f)};
    std::vector<glm::vec2> quadUV
        = {glm::vec2(1.f, 1.f),
           glm::vec2(0.f, 1.f),
           glm::vec2(0.f, 0.f),
           glm::vec2(1.f, 1.f),
           glm::vec2(0.f, 0.f),
           glm::vec2(1.f, 0.f)};
    if (x_flip)
        for (auto &uv : quadUV)
            uv.x = 1.f - uv.x;
    if (y_flip)
        for (auto &uv : quadUV)
            uv.y = 1.f - uv.y;
    vao->bind();
    vertBuffer->uploadData(sizeof(glm::vec3) * 6, quad.data(), GL_STATIC_DRAW);
    uvBuffer->uploadData(sizeof(glm::vec2) * 6, quadUV.data(), GL_STATIC_DRAW);
}

void ScreenQuad::draw() {
    vao->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}