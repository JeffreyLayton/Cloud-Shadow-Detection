#pragma once
#include "types.h"

namespace VectorGridOperations {
std::shared_ptr<VectorGrid>
GenerateVectorGrid(std::shared_ptr<ImageFloat> Zenith, std::shared_ptr<ImageFloat> Azimuth);

float SumOfSquareDistance(std::shared_ptr<VectorGrid> A, float DiagonalLength, glm::vec3 p);

struct LMSPointReturn {
    glm::vec3 p;
    bool bounded;
    float lambda;
};
LMSPointReturn LSPoint(std::shared_ptr<VectorGrid> A, float DiagonalLength);
LMSPointReturn LSPointEqualTo(std::shared_ptr<VectorGrid> A, float DiagonalLength, float z);
LMSPointReturn LSPointGreaterThan(std::shared_ptr<VectorGrid> A, float DiagonalLength, float min_z);
LMSPointReturn LSPointLessThan(std::shared_ptr<VectorGrid> A, float DiagonalLength, float max_z);

float AverageDotProduct(std::shared_ptr<VectorGrid> A, float DiagonalLength, glm::vec3 pos);
glm::vec3 AverageDirection(std::shared_ptr<VectorGrid> A);
}  // namespace VectorGridOperations
