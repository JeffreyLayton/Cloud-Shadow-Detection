#define _USE_MATH_DEFINES
#include "VectorGridOperations.h"

#include <math.h>

#include "Functions.h"
#include "ImageOperations.h"

namespace VectorGridOperations {
std::shared_ptr<VectorGrid>
GenerateVectorGrid(std::shared_ptr<ImageFloat> Zenith, std::shared_ptr<ImageFloat> Azimuth) {
    if (!ImageOperations::DIM_CHECK(Zenith, Azimuth)) return nullptr;
    std::shared_ptr<VectorGrid> ret
        = std::make_shared<VectorGrid>(Azimuth->rows(), Azimuth->cols());
    for (int i = 0; i < ret->size(); i++)
        ret->data()[i]
            = {sin(Zenith->data()[i]) * sin(Azimuth->data()[i]),
               -sin(Zenith->data()[i])
                   * cos(Azimuth->data()[i])  // Negated because the y-axis is flipped
               ,
               cos(Zenith->data()[i])};
    return ret;
}
float SumOfSquareDistance(std::shared_ptr<VectorGrid> A, float DiagonalLength, glm::vec3 p) {
    glm::vec3 d(0.f), a(0.f), dist(0.f);
    float SOSD(0.f);
    for (unsigned int i = 0u; i < (unsigned int)(A->cols()); i++) {
        for (unsigned int j = 0u; j < (unsigned int)(A->rows()); j++) {
            a    = ImageOperations::pos(A, DiagonalLength, i, j);
            d    = ImageOperations::at(A, i, j);
            dist = Functions::planeProjection(p - a, d);
            SOSD += glm::dot(dist, dist);
        }
    }
    return SOSD;
}
struct __getLSSystemReturn__ {
    glm::mat3 M;
    glm::vec3 b;
};
__getLSSystemReturn__ __getLSSystem__(std::shared_ptr<VectorGrid> A, float DiagonalLength) {
    // Someone had a similar problem, but derived differently then we did, reference only
    // https://stackoverflow.com/questions/48154210/3d-point-closest-to-multiple-lines-in-3d-space
    glm::vec3 b(0.f), d(0.f), a(0.f), row_0(0.f), row_1(0.f), row_2(0.f);
    float d_d_sum(0.f);
    for (unsigned int i = 0u; i < (unsigned int)(A->cols()); i++) {
        for (unsigned int j = 0u; j < (unsigned int)(A->rows()); j++) {
            a = ImageOperations::pos(A, DiagonalLength, i, j);
            d = glm::normalize(ImageOperations::at(A, i, j));
            if (!(isnan(a.x) || isnan(a.y) || isnan(a.z) || isnan(d.x) || isnan(d.y) || isnan(d.z)
                )) {
                d_d_sum += 1.f;
                b -= Functions::planeProjection(a, d);
                row_0 += d[0] * d;
                row_1 += d[1] * d;
                row_2 += d[2] * d;
            }
        }
    }
    glm::mat3 M = {
        {row_0.x - d_d_sum, row_1.x, row_2.x}  // Column 0
        ,
        {row_0.y, row_1.y - d_d_sum, row_2.y}  // Column 1
        ,
        {row_0.z, row_1.z, row_2.z - d_d_sum}  // Column 2
    };
    return {M, b};
};

//(NOT USED)
LMSPointReturn LSPoint(std::shared_ptr<VectorGrid> A, float DiagonalLength) {
    auto system = __getLSSystem__(A, DiagonalLength);
    return {Functions::solve(system.M, system.b), false, 0.f};
}

LMSPointReturn LSPointEqualTo(std::shared_ptr<VectorGrid> A, float DiagonalLength, float z) {
    auto system = __getLSSystem__(A, DiagonalLength);
    glm::mat4 M_4
        = {{system.M[0], 0.f}, {system.M[1], 0.f}, {system.M[2], 1.f}, {0.f, 0.f, .5f, 0.f}};
    glm::vec4 b_4    = {system.b, z};
    glm::vec4 result = Functions::solve(M_4, b_4);
    return {glm::vec3(result), true, result.w};
}

//(NOT USED)
LMSPointReturn
LSPointGreaterThan(std::shared_ptr<VectorGrid> A, float DiagonalLength, float min_z) {
    auto system = __getLSSystem__(A, DiagonalLength);
    glm::mat4 M_4
        = {{system.M[0], 0.f}, {system.M[1], 0.f}, {system.M[2], 1.f}, {0.f, 0.f, .5f, 0.f}};
    glm::vec4 b_4 = {system.b, min_z};
    // Solutions
    glm::vec3 p_3 = Functions::solve(system.M, system.b);
    glm::vec4 p_4 = Functions::solve(M_4, b_4);

    bool UnboundedValid = !(isnan(p_3[0]) || isnan(p_3[1]) || isnan(p_3[2])) && p_3.z >= min_z;
    bool BoundedValid   = !(isnan(p_4[0]) || isnan(p_4[1]) || isnan(p_4[2]) || isnan(p_4[3]));
    if (UnboundedValid && BoundedValid) {
        if (SumOfSquareDistance(A, DiagonalLength, p_3)
            <= SumOfSquareDistance(A, DiagonalLength, glm::vec3(p_4)))
            return {p_3, false, 0.f};
        else return {glm::vec3(p_4), true, p_4.w};
    } else if (UnboundedValid) return {p_3, false, 0.f};
    else if (BoundedValid) return {glm::vec3(p_4), true, p_4.w};
    else return {glm::vec3(p_4), false, p_4.w};
}

//(NOT USED)
LMSPointReturn LSPointLessThan(std::shared_ptr<VectorGrid> A, float DiagonalLength, float max_z) {
    auto system = __getLSSystem__(A, DiagonalLength);
    glm::mat4 M_4
        = {{system.M[0], 0.f}, {system.M[1], 0.f}, {system.M[2], 1.f}, {0.f, 0.f, .5f, 0.f}};
    glm::vec4 b_4 = {system.b, max_z};
    // Solutions
    glm::vec3 p_3 = Functions::solve(system.M, system.b);
    glm::vec4 p_4 = Functions::solve(M_4, b_4);

    bool UnboundedValid = !(isnan(p_3[0]) || isnan(p_3[1]) || isnan(p_3[2])) && p_3.z <= max_z;
    bool BoundedValid   = !(isnan(p_4[0]) || isnan(p_4[1]) || isnan(p_4[2]) || isnan(p_4[3]));
    if (UnboundedValid && BoundedValid) {
        if (SumOfSquareDistance(A, DiagonalLength, p_3)
            <= SumOfSquareDistance(A, DiagonalLength, glm::vec3(p_4)))
            return {p_3, false, 0.f};
        else return {glm::vec3(p_4), true, p_4.w};
    } else if (UnboundedValid) return {p_3, false, 0.f};
    else if (BoundedValid) return {glm::vec3(p_4), true, p_4.w};
    else return {glm::vec3(p_4), false, p_4.w};
}

float AverageDotProduct(std::shared_ptr<VectorGrid> A, float DiagonalLength, glm::vec3 pos) {
    glm::vec3 d(0.f), a(0.f);
    float count(0), sum(0.f);
    for (unsigned int i = 0u; i < (unsigned int)(A->cols()); i++) {
        for (unsigned int j = 0u; j < (unsigned int)(A->rows()); j++) {
            a = ImageOperations::pos(A, DiagonalLength, i, j);
            d = glm::normalize(ImageOperations::at(A, i, j));
            if (!(isnan(a[0]) || isnan(a[1]) || isnan(a[2]) || isnan(d[0]) || isnan(d[1])
                  || isnan(d[2]))) {
                count += 1.f;
                sum += glm::dot(d, glm::normalize(pos - a));
            }
        }
    }
    return sum / count;
}

glm::vec3 AverageDirection(std::shared_ptr<VectorGrid> A) { return normalize(A->mean()); }
}  // namespace VectorGridOperations