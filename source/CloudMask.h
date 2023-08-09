#pragma once
#include "types.h"

namespace CloudMask {
struct GenerateCloudMaskReturn {
    std::shared_ptr<ImageBool> cloudMask;
    std::shared_ptr<ImageFloat> blendedCloudProbability;
};
GenerateCloudMaskReturn GenerateCloudMask(
    std::shared_ptr<ImageFloat> CLP,
    std::shared_ptr<ImageFloat> CLD,
    std::shared_ptr<ImageUint> SCL
);
struct PartitionCloudMaskReturn {
    CloudQuads clouds;
    std::shared_ptr<ImageInt> map;
};
PartitionCloudMaskReturn PartitionCloudMask(
    std::shared_ptr<ImageBool> CloudMaskData,
    float DiagonalLength,
    unsigned int min_cloud_area
);
}  // namespace CloudMask
