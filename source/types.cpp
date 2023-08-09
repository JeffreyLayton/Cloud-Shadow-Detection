#include "types.h"

glm::u32 ImageBounds::size() { return (p1.x - p0.x + 1) * (p1.y - p0.y + 1); }

std::vector<glm::vec3> ImageBounds::lineStrip() {
    return {
        {p0.x, p0.y, 0.f},
        {p1.x, p0.y, 0.f},
        {p1.x, p1.y, 0.f},
        {p0.x, p1.y, 0.f},
        {p0.x, p0.y, 0.f}};
}

// ------------- LINE ---------------
Line::Line()
    : p0({0.f, 0.f, 0.f})
    , d({0.f, 0.f, 0.f}) {}
Line::Line(glm::vec3 ip0, glm::vec3 id)
    : p0(ip0)
    , d(id) {}
glm::vec3 Line::operator()(float t) { return p0 + d * t; }
glm::vec3 Line::operator<<(glm::vec3 p) { return d * glm::dot(d, p) / glm::dot(d, d); }
void Line::apply(glm::mat4 M) {
    p0 = glm::vec3(M * glm::vec4(p0, 1.f));
    d  = glm::vec3(M * glm::vec4(d, 0.f));
}
Line operator*(glm::mat4 M, Line l) {
    return Line(glm::vec3(M * glm::vec4(l.p0, 1.f)), glm::vec3(M * glm::vec4(l.d, 0.f)));
}
Line operator*(Line l, glm::mat4 M) {
    return Line(glm::vec3(glm::vec4(l.p0, 1.f) * M), glm::vec3(glm::vec4(l.d, 0.f) * M));
}
// ------------- PLANE ---------------
Plane::Plane()
    : p0({0.f, 0.f, 0.f})
    , n({0.f, 0.f, 0.f}) {}
Plane::Plane(glm::vec3 ip0, glm::vec3 in)
    : p0(ip0)
    , n(in) {}
glm::vec3 Plane::operator<<(glm::vec3 p) { return p - n * glm::dot(n, p) / glm::dot(n, n); }
void Plane::apply(glm::mat4 M) {
    p0 = glm::vec3(M * glm::vec4(p0, 1.f));
    n  = glm::vec3(M * glm::vec4(n, 0.f));
}
Plane operator*(glm::mat4 M, Plane p) {
    return Plane(glm::vec3(M * glm::vec4(p.p0, 1.f)), glm::vec3(M * glm::vec4(p.n, 0.f)));
}
Plane operator*(Plane p, glm::mat4 M) {
    return Plane(glm::vec3(glm::vec4(p.p0, 1.f) * M), glm::vec3(glm::vec4(p.n, 0.f) * M));
}

glm::vec3 operator&(Plane p, Line l) { return l(glm::dot(p.n, p.p0 - l.p0) / glm::dot(p.n, l.d)); }
glm::vec3 operator&(Line l, Plane p) { return p & l; }

// ------------- QUAD ---------------
Quad::Quad(glm::vec3 ip00, glm::vec3 ip01, glm::vec3 ip10, glm::vec3 ip11)
    : p00(ip00)
    , p01(ip01)
    , p10(ip10)
    , p11(ip11) {}
void Quad::apply(glm::mat4 M) {
    p00 = glm::vec3(M * glm::vec4(p00, 1.f));
    p01 = glm::vec3(M * glm::vec4(p01, 1.f));
    p10 = glm::vec3(M * glm::vec4(p10, 1.f));
    p11 = glm::vec3(M * glm::vec4(p11, 1.f));
}
std::vector<glm::vec3> Quad::lineStrip() { return {p00, p01, p10, p11, p00}; }
Quad operator*(glm::mat4 M, Quad q) {
    return Quad(
        glm::vec3(M * glm::vec4(q.p00, 1.f)),
        glm::vec3(M * glm::vec4(q.p01, 1.f)),
        glm::vec3(M * glm::vec4(q.p10, 1.f)),
        glm::vec3(M * glm::vec4(q.p11, 1.f))
    );
}
Quad operator*(Quad q, glm::mat4 M) {
    return Quad(
        glm::vec3(glm::vec4(q.p00, 1.f) * M),
        glm::vec3(glm::vec4(q.p01, 1.f) * M),
        glm::vec3(glm::vec4(q.p10, 1.f) * M),
        glm::vec3(glm::vec4(q.p11, 1.f) * M)
    );
}