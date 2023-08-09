#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>

class Camera2D {
  public:
    Camera2D(glm::vec2 target);

    glm::mat4 getView();
    glm::vec3 getPos();

    void incrementTarget(glm::vec2 dt);

    void setTarget(glm::vec2 t);

  private:
    glm::vec2 _target;
};

class CameraTurnTable {
  public:
    CameraTurnTable(float t, float p, float r);

    glm::mat4 getView();
    glm::vec3 getPos();
    void incrementTheta(float dt);
    void incrementPhi(float dp);
    void incrementR(float dr);

    void setTheta(float t);
    void setPhi(float p);
    void setR(float r);

  private:
    float theta;
    float phi;
    float radius;
};
