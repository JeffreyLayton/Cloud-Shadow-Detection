#include "CloudShadowMatching.h"

#include "Functions.h"
#include "ImageOperations.h"

using namespace ImageOperations;
namespace CloudShadowMatching {
struct __SimilarityComparision__Return {
    float similarity;
    ShadowQuad shadow;
};
__SimilarityComparision__Return __SimilarityComparision__(
    CloudQuad cloud,
    glm::mat4 M,
    std::shared_ptr<ImageInt> cloudMap,
    std::shared_ptr<ImageBool> cloudMask,
    std::shared_ptr<ImageBool> potentialShadow,
    float DiagonalLength
) {
    __SimilarityComparision__Return ret;
    ret.similarity              = -1.1f;
    ret.shadow.quad             = M * cloud.quad;
    ret.shadow.pixels.bounds.p0 = {Functions::nan<unsigned int>(), Functions::nan<unsigned int>()};
    ret.shadow.pixels.bounds.p1 = {Functions::nan<unsigned int>(), Functions::nan<unsigned int>()};
    ret.shadow.pixels.id        = cloud.pixels.id;

    glm::mat4 M_inverse = glm::inverse(M);

    unsigned int T = 0u;
    unsigned int C = 0u;

    glm::ivec2 p00_shadow = index(potentialShadow, DiagonalLength, ret.shadow.quad.p00);
    glm::ivec2 p01_shadow = index(potentialShadow, DiagonalLength, ret.shadow.quad.p01);
    glm::ivec2 p10_shadow = index(potentialShadow, DiagonalLength, ret.shadow.quad.p10);
    glm::ivec2 p11_shadow = index(potentialShadow, DiagonalLength, ret.shadow.quad.p11);

    unsigned int min_x_in = std::clamp(
        std::min({p00_shadow.x, p01_shadow.x, p10_shadow.x, p11_shadow.x}),
        0,
        int(potentialShadow->cols()) - 1
    );
    unsigned int min_y_in = std::clamp(
        std::min({p00_shadow.y, p01_shadow.y, p10_shadow.y, p11_shadow.y}),
        0,
        int(potentialShadow->rows()) - 1
    );
    unsigned int max_x_in = std::clamp(
        std::max({p00_shadow.x, p01_shadow.x, p10_shadow.x, p11_shadow.x}),
        0,
        int(potentialShadow->cols()) - 1
    );
    unsigned int max_y_in = std::clamp(
        std::max({p00_shadow.y, p01_shadow.y, p10_shadow.y, p11_shadow.y}),
        0,
        int(potentialShadow->rows()) - 1
    );

    unsigned int min_x_out = std::numeric_limits<unsigned int>::max();
    unsigned int min_y_out = std::numeric_limits<unsigned int>::max();
    unsigned int max_x_out = 0u;
    unsigned int max_y_out = 0u;

    float W = cloudMap->cols();
    float H = cloudMap->rows();

    float ratio   = DiagonalLength / sqrtf(W * W + H * H);
    float ratio_r = 1.f / ratio;

    glm::ivec2 shadow_pixel;
    glm::vec2 delta = M_inverse * glm::vec4(.5f, .5f, 0.f, ratio_r);
    glm::mat2 M2    = glm::mat2(M_inverse);

    glm::uvec2 pixel;
    for (pixel.x = min_x_in; pixel.x <= max_x_in; pixel.x++) {
        for (pixel.y = min_y_in; pixel.y <= max_y_in; pixel.y++) {
            if (!at(cloudMask, pixel.x, pixel.y)) {
                // If the pixel in shadow space isnt a cloud
                shadow_pixel = M2 * pixel + delta;
                if (valid(cloudMap, shadow_pixel.x, shadow_pixel.y)) {
                    // If the pixel is under the desired cloud
                    if (at(cloudMap, shadow_pixel.x, shadow_pixel.y) == cloud.pixels.id) {
                        // If the pixel is indeed a shadow
                        T++;
                        if (at(potentialShadow, pixel.x, pixel.y)) {
                            C++;
                            ret.shadow.pixels.list.push_back(pixel);
                            min_x_out = std::min(min_x_out, pixel.x);
                            min_y_out = std::min(min_y_out, pixel.y);
                            max_x_out = std::max(max_x_out, pixel.x);
                            max_y_out = std::max(max_y_out, pixel.y);
                        }
                    }
                }
            }
        }
    }
    if (T < 5) {
        ret.similarity = -1.1f;
        ret.shadow.pixels.list.clear();
    } else {
        ret.similarity = float(C) / float(T);
        ret.shadow.quad.p00
            = pos(cloudMask, DiagonalLength, min_x_out, min_y_out, .1f, .1f);  // p11---p10
        ret.shadow.quad.p01
            = pos(cloudMask, DiagonalLength, max_x_out, min_y_out, .9f, .1f);  //  |<<<<<|
        ret.shadow.quad.p10
            = pos(cloudMask, DiagonalLength, max_x_out, max_y_out, .9f, .9f);  //  |>>>>>|
        ret.shadow.quad.p11
            = pos(cloudMask, DiagonalLength, min_x_out, max_y_out, .1f, .9f);  // p00---p01
        ret.shadow.pixels.bounds.p0 = glm::uvec2(min_x_out, min_y_out);
        ret.shadow.pixels.bounds.p1 = glm::uvec2(max_x_out, max_y_out);
    }
    return ret;
}
struct __MatchCloudShadow__Ret {
    OptimalSolution solution;
    ShadowQuad shadow;
};
__MatchCloudShadow__Ret __MatchCloudShadow__(
    CloudQuad cloud,
    std::shared_ptr<ImageInt> cloudMap,
    std::shared_ptr<ImageBool> cloudMask,
    std::shared_ptr<ImageBool> potentialShadow,
    float DiagonalLength,
    glm::vec3 sunPos,
    glm::vec3 viewPos
) {
    __MatchCloudShadow__Ret ret;
    ret.solution.similarity     = -1.f;
    ret.solution.height         = 0.f;
    ret.solution.M              = glm::mat4(1.f);
    ret.solution.id             = cloud.pixels.id;
    ret.shadow.quad             = cloud.quad;
    ret.shadow.pixels.bounds.p0 = {Functions::nan<unsigned int>(), Functions::nan<unsigned int>()};
    ret.shadow.pixels.bounds.p1 = {Functions::nan<unsigned int>(), Functions::nan<unsigned int>()};
    ret.shadow.pixels.id        = cloud.pixels.id;
    // Rest have default constructors
    Quad casted;
    glm::mat4 M;
    Plane height_plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f});
    Plane ground_plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f});
    __SimilarityComparision__Return sim_ret;
    for (height_plane.p0.z = .2f; height_plane.p0.z <= 12.f; height_plane.p0.z += .025f) {
        casted  = Functions::perspective(cloud.quad, viewPos, height_plane);
        casted  = Functions::perspective(casted, sunPos, ground_plane);
        M       = Functions::affineTransform(cloud.quad, casted);
        M[2][2] = 1.f;  // Make matrix invertable by leaving z direction identity
        sim_ret = __SimilarityComparision__(
            cloud, M, cloudMap, cloudMask, potentialShadow, DiagonalLength
        );
        if (sim_ret.similarity > ret.solution.similarity) {
            ret.solution.similarity = sim_ret.similarity;
            ret.solution.height     = height_plane.p0.z;
            ret.solution.M          = M;
            ret.shadow              = sim_ret.shadow;
        }
    }
    // Must be at least this similar to count
    if (ret.solution.similarity < .3f) {
        ret.solution.similarity = -1.f;
        ret.solution.height = 0.f;  // Not actually 0 but at this height, it will be treated as NULL
        ret.solution.M      = glm::mat4(1.f);
        ret.shadow.quad     = cloud.quad;
        ret.shadow.pixels.bounds.p0
            = {Functions::nan<unsigned int>(), Functions::nan<unsigned int>()};
        ret.shadow.pixels.bounds.p1
            = {Functions::nan<unsigned int>(), Functions::nan<unsigned int>()};
        ret.shadow.pixels.list.clear();
    }
    return ret;
}

MatchCloudsShadowsResults MatchCloudsShadows(
    CloudQuads clouds,
    std::shared_ptr<ImageInt> cloudMap,
    std::shared_ptr<ImageBool> cloudMask,
    std::shared_ptr<ImageBool> potentialShadow,
    float DiagonalLength,
    glm::vec3 sunPos,
    glm::vec3 viewPos
) {
    MatchCloudsShadowsResults ret;
    ret.trimmedMeanHeight = 0.f;
    ret.shadowMask        = std::make_shared<ImageBool>(cloudMask->rows(), cloudMask->cols());
    ret.shadowMask->fill(false);
    std::vector<float> heights;
    heights.reserve(clouds.size());
    for (auto &c : clouds) {
        __MatchCloudShadow__Ret sol = __MatchCloudShadow__(
            c.second, cloudMap, cloudMask, potentialShadow, DiagonalLength, sunPos, viewPos
        );
        ret.solutions.insert({c.first, sol.solution});
        ret.shadows.insert({c.first, sol.shadow});
        for (auto &p : sol.shadow.pixels.list)
            set(ret.shadowMask, p.x, p.y, true);
        // Only Valid Heights
        if (sol.solution.height >= .2f) heights.push_back(sol.solution.height);
    }
    // Only use the Middle 80%
    ret.trimmedMeanHeight = Functions::trimmedAverage(heights, 0.1f, 0.9f);
    return ret;
}
}  // namespace CloudShadowMatching