#pragma once
#include <filesystem>
#include <map>

#include <Eigen/Core>

#include "glm/glm.hpp"

#ifndef SHADOW_DETECTION_TYPE_H
#    define SHADOW_DETECTION_TYPE_H

using Path = ::std::filesystem::path;

template<typename T>
using Image      = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using ImageFloat = Image<float>;
using ImageBool  = Image<bool>;
using ImageInt   = Image<int>;
using ImageUint  = Image<unsigned int>;
using VectorGrid = Image<glm::vec3>;
struct ImageBounds {
    glm::uvec2 p0;
    glm::uvec2 p1;
    glm::u32 size();
    std::vector<glm::vec3> lineStrip();
};
struct Pixels {
    ImageBounds bounds;
    std::vector<glm::uvec2> list;
    int id;
};

struct Quad {
    Quad() = default;
    Quad(glm::vec3 ip00, glm::vec3 ip01, glm::vec3 ip10, glm::vec3 ip11);
    void apply(glm::mat4 M);
    std::vector<glm::vec3> lineStrip();
    glm::vec3 p00, p01, p10, p11;
};
Quad operator*(glm::mat4 M, Quad q);
Quad operator*(Quad q, glm::mat4 M);

struct PixelsQuad {
    Pixels pixels;
    Quad quad;
};

using Cloud      = Pixels;
using Clouds     = std::map<int, Cloud>;
using CloudQuad  = PixelsQuad;
using CloudQuads = std::map<int, CloudQuad>;

using Shadow      = Pixels;
using Shadows     = std::map<int, Shadow>;
using ShadowQuad  = PixelsQuad;
using ShadowQuads = std::map<int, ShadowQuad>;

struct Line {
    Line();
    Line(glm::vec3 ip0, glm::vec3 id);
    glm::vec3 operator()(float t);
    glm::vec3 operator<<(glm::vec3 p);
    void apply(glm::mat4 M);
    glm::vec3 p0, d;
};
Line operator*(glm::mat4 M, Line l);
Line operator*(Line l, glm::mat4 M);

struct Plane {
    Plane();
    Plane(glm::vec3 ip0, glm::vec3 in);
    glm::vec3 operator<<(glm::vec3 p);
    void apply(glm::mat4 M);
    glm::vec3 p0, n;
};
Plane operator*(glm::mat4 M, Plane p);
Plane operator*(Plane p, glm::mat4 M);

glm::vec3 operator&(Plane p, Line l);
glm::vec3 operator&(Line l, Plane p);

#endif  // !SHADOW_DETECTION_TYPE_H
