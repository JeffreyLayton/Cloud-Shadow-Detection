#pragma once

#include <memory>
#include <vector>

#include "types.h"

namespace GaussianBlur {
void init();
std::vector<float> StripKernel(float sigma);
std::shared_ptr<ImageFloat> GaussianBlurFilter(std::shared_ptr<ImageFloat> in, float sigma);
};  // namespace GaussianBlur
