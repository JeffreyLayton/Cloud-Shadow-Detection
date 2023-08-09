#pragma once
#include <memory>

#include "types.h"

namespace PotentialShadowMask {
struct PotentialShadowMaskGenerationReturn {
    std::shared_ptr<ImageBool> mask;
    std::shared_ptr<ImageFloat> difference_of_pitfill_NIR;
};
PotentialShadowMaskGenerationReturn GeneratePotentialShadowMask(
    std::shared_ptr<ImageFloat> NIR,
    std::shared_ptr<ImageBool> CloudMask,
    std::shared_ptr<ImageUint> SCL
);
}  // namespace PotentialShadowMask
