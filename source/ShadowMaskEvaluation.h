#pragma once
#include "types.h"

namespace ShadowMaskEvaluation {
static const unsigned int NO_DATA_COLOUR        = 0xff000000;  // BLACK
static const unsigned int TRUE_NEGATIVE_COLOUR  = 0xff00ff00;  // GREEN
static const unsigned int TRUE_POSITIVE_COLOUR  = 0xffff0000;  // BLUE
static const unsigned int FALSE_NEGATIVE_COLOUR = 0xff0000ff;  // RED
static const unsigned int FALSE_POSITIVE_COLOUR = 0xffff00ff;  // PINK
static const unsigned int CLOUD_COLOUR          = 0xffffffff;  // WHITE

struct Results {
    static const unsigned int unknown_class_value        = 0u;
    static const unsigned int true_negative_class_value  = 1u;
    static const unsigned int true_positive_class_value  = 2u;
    static const unsigned int false_negative_class_value = 3u;
    static const unsigned int false_positive_class_value = 4u;
    static const unsigned int clouds_class_value         = 5u;

    std::shared_ptr<ImageUint> pixel_classes = nullptr;
    float positive_error_total               = 0.f;
    float negative_error_total               = 0.f;
    float error_total                        = 0.f;
    float positive_error_relative            = 0.f;
    float negative_error_relative            = 0.f;
    float error_relative                     = 0.f;
    float producers_accuracy                 = 0.f;
    float users_accuracy                     = 0.f;
};

Results Evaluate(
    std::shared_ptr<ImageBool> shadow_mask,
    std::shared_ptr<ImageBool> cloud_mask,
    std::shared_ptr<ImageBool> shadow_baseline,
    ImageBounds evaluation_bounds
);
std::shared_ptr<ImageUint> GenerateRGBA(std::shared_ptr<ImageUint> A);

ImageBounds CastedImageBounds(
    std::shared_ptr<ImageBool> mask,
    float DiagonalLength,
    glm::vec3 sun_pos,
    glm::vec3 view_pos,
    float height
);
};  // namespace ShadowMaskEvaluation
