#include "PotentialShadowMask.h"

#include "Functions.h"
#include "GaussianBlur.h"
#include "ImageOperations.h"
#include "PitFillAlgorithm.h"
#include "SceneClassificationLayer.h"

using namespace ImageOperations;
using namespace GaussianBlur;
using namespace SceneClassificationLayer;
using namespace PitFillAlgorithm;
using namespace Functions;

PotentialShadowMask::PotentialShadowMaskGenerationReturn
PotentialShadowMask::GeneratePotentialShadowMask(
    std::shared_ptr<ImageFloat> NIR,
    std::shared_ptr<ImageBool> CloudMask,
    std::shared_ptr<ImageUint> SCL
) {
    std::shared_ptr<ImageBool> SCL_SHADOW_DARK
        = GenerateMask(SCL, CLOUD_SHADOWS_MASK | DARK_AREA_PIXELS_MASK);
    std::shared_ptr<ImageBool> SCL_SHADOW_DARK_WATER
        = GenerateMask(SCL, CLOUD_SHADOWS_MASK | DARK_AREA_PIXELS_MASK | WATER_MASK);
    std::vector<float> ClearSky_NIR_Values
        = partitionUnobscuredObscured(NIR, OR(CloudMask, SCL_SHADOW_DARK_WATER))->first;
    float CloudCover_percent   = CoverPercentage(CloudMask);
    float ClearSky_NIR_percent = linearStep(CloudCover_percent, {.07f, .2f}, {.4f, .7f});
    float Outside_value        = percentile(ClearSky_NIR_Values, ClearSky_NIR_percent);
    std::shared_ptr<ImageFloat> NIR_pitfilled     = PitFillAlgorithmFilter(NIR, Outside_value);
    std::shared_ptr<ImageFloat> NIR_difference    = SUBTRACT(NIR_pitfilled, NIR);
    std::shared_ptr<ImageBool> NIR_prelim_mask    = Threshold(NIR_difference, .12f);
    std::shared_ptr<ImageBool> Result_prelim_mask = Threshold(
        GaussianBlurFilter(cast<float, bool>(OR(NIR_prelim_mask, SCL_SHADOW_DARK)), 1.f), 0.1f
    );
    std::shared_ptr<ImageBool> Result_mask = AND(NOT(CloudMask), Result_prelim_mask);
    return {Result_mask, NIR_difference};
}