#include "Camera.h"

#define _USE_MATH_DEFINES
#include <algorithm>

#include <math.h>

#include "glm/gtc/matrix_transform.hpp"

Camera2D::Camera2D(glm::vec2 target)
    : _target(target) {}

glm::mat4 Camera2D::getView() {
    glm::vec3 eye = glm::vec3(_target, 1.0f);
    glm::vec3 at  = glm::vec3(_target, 0.0f);
    glm::vec3 up  = glm::vec3(0.0f, 1.0f, 0.0f);
    return glm::lookAt(eye, at, up);
}

glm::vec3 Camera2D::getPos() { return glm::vec3(_target, 1.0f); }

void Camera2D::incrementTarget(glm::vec2 dt) { _target += dt; }

void Camera2D::setTarget(glm::vec2 t) { _target = t; }

CameraTurnTable::CameraTurnTable(float t, float p, float r)
    : theta(t)
    , phi(p)
    , radius(r) {}

glm::mat4 CameraTurnTable::getView() {
    glm::vec3 eye = radius
        * glm::vec3(std::cos(theta) * std::sin(phi),
                    std::sin(theta),
                    std::cos(theta) * std::cos(phi));
    glm::vec3 at = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    return glm::lookAt(eye, at, up);
}

glm::vec3 CameraTurnTable::getPos() {
    return radius
        * glm::vec3(
               std::cos(theta) * std::sin(phi), std::sin(theta), std::cos(theta) * std::cos(phi)
        );
}

void CameraTurnTable::incrementTheta(float dt) {
    if (theta + dt < M_PI_2 && theta + dt > -M_PI_2) { theta += dt; }
}

void CameraTurnTable::incrementPhi(float dp) {
    phi -= dp;
    if (phi > 2.f * M_PI) {
        phi -= 2.f * M_PI;
    } else if (phi < 0.f) {
        phi += 2.f * M_PI;
    }
}

void CameraTurnTable::incrementR(float dr) { radius -= dr; }

void CameraTurnTable::setTheta(float t) { theta = std::clamp(t, -float(M_PI_2), float(M_PI_2)); }

void CameraTurnTable::setPhi(float p) { phi = p; }

void CameraTurnTable::setR(float r) { radius = r; }