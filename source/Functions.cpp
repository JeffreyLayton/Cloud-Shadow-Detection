#include <algorithm>
#include <queue>

#include "Functions.h"

#include <Eigen/Core>
#include <Eigen/QR>

bool Functions::equal(std::string &s1, std::string &s2) { return s1.compare(s2) == 0; }
bool Functions::equal(std::string &s1, char *s2) { return s1.compare(s2) == 0; }
bool Functions::equal(std::string &s1, const char *s2) { return s1.compare(s2) == 0; }
bool Functions::equal(char *s1, std::string &s2) { return s2.compare(s1) == 0; }
bool Functions::equal(const char *s1, std::string &s2) { return s2.compare(s1) == 0; }

bool Functions::equal(float x, float y, float eps) { return abs(x - y) > eps; }

float Functions::linearStep(float x, glm::vec2 p0, glm::vec2 p1) {
    if (p0.x > p1.x) std::swap(p0, p1);
    if (x < p0.x) return p0.y;
    else if (x > p1.x) return p1.y;
    else return p0.y + (x - p0.x) * (p1.y - p0.y) / (p1.x - p0.x);
}

float Functions::percentile(std::vector<float> collection, float percent) {
    sort(collection.begin(), collection.end());
    unsigned int x = (unsigned int)(percent * float(collection.size()));
    return (x < 1) ? 0.f : (x > collection.size()) ? 1.f : collection[size_t(x) - 1];
}

float Functions::distance(glm::vec2 p0, glm::vec2 p1) {
    static const float R = 6371.f;  // Radius of the earth in km
    p0                   = glm::radians(p0);
    p1                   = glm::radians(p1);
    glm::vec2 dLonLat_2  = (p1 - p0) * .5f;
    float a              = sinf(dLonLat_2.y) * sinf(dLonLat_2.y)
        + cosf(p0.y) * cosf(p0.y) * sinf(dLonLat_2.x) * sinf(dLonLat_2.x);
    return 2.f * R * atan2f(sqrtf(a), sqrtf(1 - a));
}

glm::vec3 Functions::lineProjection(glm::vec3 v, glm::vec3 d_hat) {
    return d_hat * glm::dot(d_hat, v);
}

glm::vec3 Functions::planeProjection(glm::vec3 v, glm::vec3 n_hat) {
    return v - lineProjection(v, n_hat);
}

Quad Functions::perspective(Quad q, glm::vec3 eye, Plane plane) {
    return Quad(
        plane & Line(q.p00, eye - q.p00),
        plane & Line(q.p01, eye - q.p01),
        plane & Line(q.p10, eye - q.p10),
        plane & Line(q.p11, eye - q.p11)
    );
}

glm::mat4 Functions::affineTransform(Quad qi, Quad qf) {
    Eigen::Matrix<long double, 4, 4, Eigen::RowMajor> x1, x2, M;
    x1.col(0) << qi.p00.x, qi.p00.y, qi.p00.z, 1.l;
    x1.col(1) << qi.p01.x, qi.p01.y, qi.p01.z, 1.l;
    x1.col(2) << qi.p10.x, qi.p10.y, qi.p10.z, 1.l;
    x1.col(3) << qi.p11.x, qi.p11.y, qi.p11.z, 1.l;

    x2.col(0) << qf.p00.x, qf.p00.y, qf.p00.z, 1.l;
    x2.col(1) << qf.p01.x, qf.p01.y, qf.p01.z, 1.l;
    x2.col(2) << qf.p10.x, qf.p10.y, qf.p10.z, 1.l;
    x2.col(3) << qf.p11.x, qf.p11.y, qf.p11.z, 1.l;

    M = x2 * x1.fullPivHouseholderQr().inverse();

    return {
        {M.col(0)(0), M.col(0)(1), M.col(0)(2), M.col(0)(3)},  // Column 0
        {M.col(1)(0), M.col(1)(1), M.col(1)(2), M.col(1)(3)},  // Column 1
        {M.col(2)(0), M.col(2)(1), M.col(2)(2), M.col(2)(3)},  // Column 2
        {M.col(3)(0), M.col(3)(1), M.col(3)(2), M.col(3)(3)}  // Column 3
    };
}
float Functions::triangleArea(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    return .5f * glm::length(glm::cross(p3 - p1, p2 - p1));
}

glm::vec3 Functions::barycentricCoordinates(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 p) {
    // https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
    // http://realtimecollisiondetection.net/
    glm::vec3 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00   = glm::dot(v0, v0);
    float d01   = glm::dot(v0, v1);
    float d11   = glm::dot(v1, v1);
    float d20   = glm::dot(v2, v0);
    float d21   = glm::dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    float v     = (d11 * d20 - d01 * d21) / denom;
    float w     = (d00 * d21 - d01 * d20) / denom;
    float u     = 1.0f - v - w;
    return {u, v, w};
}

bool Functions::inXY(Quad q, glm::vec2 p) {
    glm::vec3 p_z = {p.x, p.y, 0.f};
    static Plane XY_PLANE({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f});
    q = Quad(XY_PLANE << q.p00, XY_PLANE << q.p01, XY_PLANE << q.p10, XY_PLANE << q.p11);
    glm::vec3 lower = barycentricCoordinates(q.p00, q.p01, q.p10, p_z);
    glm::vec3 upper = barycentricCoordinates(q.p00, q.p10, q.p11, p_z);
    return (lower.x >= 0.f && lower.y >= 0.f && lower.z >= 0.f && lower.x <= 1.f && lower.y <= 1.f
            && lower.z <= 1.f)
        || (upper.x >= 0.f && upper.y >= 0.f && upper.z >= 0.f && upper.x <= 1.f && upper.y <= 1.f
            && upper.z <= 1.f);
}

bool Functions::in(Quad q, glm::vec3 p) { return false; }

Pixels Functions::border(Pixels p) {
    std::shared_ptr<ImageBool> map = std::make_shared<ImageBool>(
        p.bounds.p1.y - p.bounds.p0.y + 1, p.bounds.p1.x - p.bounds.p0.x + 1
    );
    map->fill(false);
#define set(i, j, v) map->operator()(j - p.bounds.p0.y, i - p.bounds.p0.x) = v;
#define get(i, j) map->operator()(j - p.bounds.p0.y, i - p.bounds.p0.x);
    for (auto &pix : p.list)
        set(pix.x, pix.y, true);
    Pixels ret;
    ret.id     = p.id;
    ret.bounds = p.bounds;
    for (auto &pix : p.list) {
        bool v_up    = get(pix.x, (unsigned int)(std::min(int(pix.y) + 1, int(p.bounds.p1.y))));
        bool v_down  = get(pix.x, (unsigned int)(std::max(int(pix.y) - 1, int(p.bounds.p0.y))));
        bool v_left  = get((unsigned int)(std::max(int(pix.x) - 1, int(p.bounds.p0.x))), pix.y);
        bool v_right = get((unsigned int)(std::min(int(pix.x) + 1, int(p.bounds.p1.x))), pix.y);
        bool edge = (pix.x == p.bounds.p0.x) || (pix.y == p.bounds.p0.y) || (pix.x == p.bounds.p1.x)
            || (pix.y == p.bounds.p1.y);
        if ((!v_up) || (!v_down) || (!v_left) || (!v_right) || edge) ret.list.push_back(pix);
    }
    return ret;
}

float Functions::quadraticRadialBasis(float d, float min, float max, float percent) {
    float a = (percent)*max + (1.f - percent) * min;
    if (d <= min) return 1.f;
    else if (d <= a) return 1.f - (d - min) * (d - min) / ((max - min) * (max - min) * (percent));
    else if (d <= max)
        return 0.f + (d - max) * (d - max) / ((max - min) * (max - min) * (1.f - percent));
    else /*(d >  max)*/ return 0.f;
}

float Functions::pixelDistance(glm::uvec2 p0, glm::uvec2 p1) {
    int dx = p0.x - p1.x;
    int dy = p0.y - p1.y;
    return sqrtf(dx * dx + dy * dy);
    // glm::length(glm::vec2(p0) - glm::vec2(p1));
}

float Functions::linear(float L, float R, float u) { return (1.f - u) * L + (u)*R; }

float Functions::bilinear(float B_L, float B_R, float T_L, float T_R, float u, float v) {
    return linear(linear(B_L, B_R, u), linear(T_L, T_R, u), v);
}

glm::vec3 Functions::solve(glm::mat3 M, glm::vec3 b) {
    Eigen::Matrix3d eM;
    Eigen::Vector3d eb;

    eM.row(0) << M[0].x, M[1].x, M[2].x;
    eM.row(1) << M[0].y, M[1].y, M[2].y;
    eM.row(2) << M[0].z, M[1].z, M[2].z;

    eb << b.x, b.y, b.z;

    Eigen::Vector3d ex = eM.colPivHouseholderQr().solve(eb);

    return {ex.x(), ex.y(), ex.z()};
}

glm::vec4 Functions::solve(glm::mat4 M, glm::vec4 b) {
    Eigen::Matrix4d eM;
    Eigen::Vector4d eb;

    eM.row(0) << M[0].x, M[1].x, M[2].x, M[3].x;
    eM.row(1) << M[0].y, M[1].y, M[2].y, M[3].y;
    eM.row(2) << M[0].z, M[1].z, M[2].z, M[3].z;
    eM.row(3) << M[0].w, M[1].w, M[2].w, M[3].w;

    eb << b.x, b.y, b.z, b.w;

    Eigen::Vector4d ex = eM.colPivHouseholderQr().solve(eb);

    return {ex.x(), ex.y(), ex.z(), ex.w()};
}

float Functions::trimmedAverage(
    std::vector<float> values,
    float min_percentile,
    float max_percentile
) {
    size_t min_index = std::max(size_t(floorf(min_percentile * float(values.size()))), size_t(0));
    size_t max_index
        = std::min(size_t(ceilf(max_percentile * float(values.size()))), size_t(values.size() - 1));
    if (min_index > max_index) return std::numeric_limits<float>::quiet_NaN();
    std::sort(values.begin(), values.end());
    float ret = 0.f;
    for (size_t i = min_index; i <= max_index; i++)
        ret += values[i];
    return ret / float(max_index - min_index + 1);
}