#pragma once
#include "types.h"

namespace CloudShadowMatching {
struct OptimalSolution {
    float height;
    float similarity;
    glm::mat4 M;
    int id;
};
struct MatchCloudsShadowsResults {
    std::map<int, OptimalSolution> solutions;
    float trimmedMeanHeight;
    ShadowQuads shadows;
    std::shared_ptr<ImageBool> shadowMask;
};
MatchCloudsShadowsResults MatchCloudsShadows(
    CloudQuads clouds,
    std::shared_ptr<ImageInt> cloudMap,
    std::shared_ptr<ImageBool> cloudMask,
    std::shared_ptr<ImageBool> potentialShadow,
    float DiagonalLength,
    glm::vec3 sunPos,
    glm::vec3 viewPos
);
}  // namespace CloudShadowMatching
