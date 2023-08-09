#include "CloudMask.h"

#include "GaussianBlur.h"
#include "ImageOperations.h"
#include "SceneClassificationLayer.h"

using namespace ImageOperations;
using namespace GaussianBlur;
using namespace SceneClassificationLayer;

CloudMask::GenerateCloudMaskReturn CloudMask::GenerateCloudMask(
    std::shared_ptr<ImageFloat> CLP,
    std::shared_ptr<ImageFloat> CLD,
    std::shared_ptr<ImageUint> SCL
) {
    CloudMask::GenerateCloudMaskReturn ret;
    ret.blendedCloudProbability = GaussianBlurFilter(CLP, 4.f);
    ret.cloudMask               = Threshold(
        GaussianBlurFilter(
            cast<float, bool>(
                OR(AND(Threshold(ret.blendedCloudProbability, .5f), Threshold(CLD, .2f)),
                   GenerateMask(SCL, CLOUD_LOW_MASK | CLOUD_MEDIUM_MASK | CLOUD_HIGH_MASK))
            ),
            1.f
        ),
        .1f
    );
    return ret;
}

CloudMask::PartitionCloudMaskReturn CloudMask::PartitionCloudMask(
    std::shared_ptr<ImageBool> CloudMaskData,
    float DiagonalLength,
    unsigned int min_cloud_area
) {
    PartitionCloudMaskReturn ret;
    ret.map = std::make_shared<ImageInt>(CloudMaskData->rows(), CloudMaskData->cols());
    ret.map->fill(-1);
    std::vector<glm::uvec2> current_cloud_pixels;
    CloudQuad cloud_temp;
    for (int i = 0, CN = 0; i < ret.map->cols(); i++) {
        for (int j = 0; j < ret.map->rows(); j++) {
            if (at(CloudMaskData, i, j) && at(ret.map, i, j) < 0) {  // Unassigned Cloud pixels
                current_cloud_pixels = flood(CloudMaskData, i, j);
                if (current_cloud_pixels.size()
                    >= min_cloud_area) {  // Large enough to be counted as a cloud object
                    int min_x = std::numeric_limits<int>::max();
                    int min_y = std::numeric_limits<int>::max();
                    int max_x = std::numeric_limits<int>::min();
                    int max_y = std::numeric_limits<int>::min();
                    for (auto &p : current_cloud_pixels) {
                        set(ret.map, p.x, p.y, CN);
                        min_x = std::min(min_x, int(p.x));
                        max_x = std::max(max_x, int(p.x));
                        ;
                        min_y = std::min(min_y, int(p.y));
                        max_y = std::max(max_y, int(p.y));
                        ;
                    }
                    cloud_temp.pixels.list      = current_cloud_pixels;
                    cloud_temp.pixels.bounds.p0 = glm::uvec2(min_x, min_y);
                    cloud_temp.pixels.bounds.p1 = glm::uvec2(max_x, max_y);
                    cloud_temp.pixels.id        = CN++;
                    cloud_temp.quad.p00
                        = pos(CloudMaskData, DiagonalLength, min_x, min_y, .1f, .1f);  // p11---p10
                    cloud_temp.quad.p01
                        = pos(CloudMaskData, DiagonalLength, max_x, min_y, .9f, .1f);  //  |<<<<<|
                    cloud_temp.quad.p10
                        = pos(CloudMaskData, DiagonalLength, max_x, max_y, .9f, .9f);  //  |>>>>>|
                    cloud_temp.quad.p11
                        = pos(CloudMaskData, DiagonalLength, min_x, max_y, .1f, .9f);  // p00---p01
                    ret.clouds.insert({cloud_temp.pixels.id, cloud_temp});
                }
            }
        }
    }
    return ret;
}