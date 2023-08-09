#include "ShadowMaskEvaluation.h"

#include "Functions.h"
#include "ImageOperations.h"

using namespace ImageOperations;

namespace ShadowMaskEvaluation {
Results Evaluate(
    std::shared_ptr<ImageBool> shadow_mask,
    std::shared_ptr<ImageBool> cloud_mask,
    std::shared_ptr<ImageBool> shadow_baseline,
    ImageBounds evaluation_bounds
) {
    Results ret;

    std::shared_ptr<ImageBool> not_cloud_mask      = NOT(cloud_mask);
    std::shared_ptr<ImageBool> not_shadow_mask     = NOT(shadow_mask);
    std::shared_ptr<ImageBool> not_shadow_baseline = NOT(shadow_baseline);

    std::shared_ptr<ImageBool> valid_shadow_mask         = AND(shadow_mask, not_cloud_mask);
    std::shared_ptr<ImageBool> valid_shadow_baseline     = AND(shadow_baseline, not_cloud_mask);
    std::shared_ptr<ImageBool> valid_not_shadow_mask     = AND(not_shadow_mask, not_cloud_mask);
    std::shared_ptr<ImageBool> valid_not_shadow_baseline = AND(not_shadow_baseline, not_cloud_mask);

    std::shared_ptr<ImageBool> valid_true_positives = AND(valid_shadow_mask, valid_shadow_baseline);
    std::shared_ptr<ImageBool> valid_true_negatives
        = AND(valid_not_shadow_mask, valid_not_shadow_baseline);
    std::shared_ptr<ImageBool> valid_false_positives
        = AND(valid_shadow_mask, valid_not_shadow_baseline);
    std::shared_ptr<ImageBool> valid_false_negatives
        = AND(valid_not_shadow_mask, valid_shadow_baseline);

    std::shared_ptr<ImageBool> valid_shadow_pixels = OR(valid_shadow_mask, valid_shadow_baseline);

    float n_total_pixels_valid    = float(evaluation_bounds.size());
    float n_relative_pixels_valid = float(SubCoverCount(valid_shadow_pixels, evaluation_bounds));
    float n_false_positives       = float(SubCoverCount(valid_false_positives, evaluation_bounds));
    float n_false_negatives       = float(SubCoverCount(valid_false_negatives, evaluation_bounds));
    float n_false                 = n_false_positives + n_false_negatives;

    ret.positive_error_total = n_false_positives / n_total_pixels_valid;
    ret.negative_error_total = n_false_negatives / n_total_pixels_valid;
    ret.error_total          = n_false / n_total_pixels_valid;

    ret.positive_error_relative = n_false_positives / n_relative_pixels_valid;
    ret.negative_error_relative = n_false_negatives / n_relative_pixels_valid;
    ret.error_relative          = n_false / n_relative_pixels_valid;

    ret.producers_accuracy = (1.f - ret.error_relative) / (1.f - ret.positive_error_relative);
    ret.users_accuracy     = (1.f - ret.error_relative) / (1.f - ret.negative_error_relative);

    std::shared_ptr<ImageUint> uint_true_negatives = cast(
        valid_true_negatives, Results::true_negative_class_value, Results::unknown_class_value
    );
    std::shared_ptr<ImageUint> uint_true_positives = cast(
        valid_true_positives, Results::true_positive_class_value, Results::unknown_class_value
    );
    std::shared_ptr<ImageUint> uint_false_negatives = cast(
        valid_false_negatives, Results::false_negative_class_value, Results::unknown_class_value
    );
    std::shared_ptr<ImageUint> uint_false_positives = cast(
        valid_false_positives, Results::false_positive_class_value, Results::unknown_class_value
    );
    std::shared_ptr<ImageUint> uint_clouds
        = cast(cloud_mask, Results::clouds_class_value, Results::unknown_class_value);

    ret.pixel_classes = ADD(
        uint_true_negatives,
        ADD(uint_true_positives, ADD(uint_false_negatives, ADD(uint_false_positives, uint_clouds)))
    );

    return ret;
}

std::shared_ptr<ImageUint> GenerateRGBA(std::shared_ptr<ImageUint> A) {
    std::shared_ptr<ImageUint> ret = std::make_shared<ImageUint>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        switch (A->data()[i]) {
            case Results::true_negative_class_value: ret->data()[i] = TRUE_NEGATIVE_COLOUR; break;
            case Results::true_positive_class_value: ret->data()[i] = TRUE_POSITIVE_COLOUR; break;
            case Results::false_negative_class_value: ret->data()[i] = FALSE_NEGATIVE_COLOUR; break;
            case Results::false_positive_class_value: ret->data()[i] = FALSE_POSITIVE_COLOUR; break;
            case Results::clouds_class_value: ret->data()[i] = CLOUD_COLOUR; break;
            default: ret->data()[i] = NO_DATA_COLOUR; break;
        }
    return ret;
}

ImageBounds CastedImageBounds(
    std::shared_ptr<ImageBool> mask,
    float DiagonalLength,
    glm::vec3 sun_pos,
    glm::vec3 view_pos,
    float height
) {
    Plane height_plane({0.f, 0.f, height}, {0.f, 0.f, 1.f});
    Plane ground_plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f});

    Quad quad;
    quad.p00 = pos(mask, DiagonalLength, 0, 0, .1f, .1f);  // p11---p10
    quad.p01 = pos(mask, DiagonalLength, mask->cols() - 1, 0, .9f, .1f);  //  |<<<<<|
    quad.p10 = pos(mask, DiagonalLength, mask->cols() - 1, mask->rows() - 1, .9f, .9f);  //  |>>>>>|
    quad.p11 = pos(mask, DiagonalLength, 0, mask->rows() - 1, .1f, .9f);  // p00---p01

    quad = Functions::perspective(quad, view_pos, height_plane);
    quad = Functions::perspective(quad, sun_pos, ground_plane);

    glm::ivec2 p00_index = index(mask, DiagonalLength, quad.p00);
    glm::ivec2 p01_index = index(mask, DiagonalLength, quad.p01);
    glm::ivec2 p10_index = index(mask, DiagonalLength, quad.p10);
    glm::ivec2 p11_index = index(mask, DiagonalLength, quad.p11);

    int min_x = std::min(std::min(p00_index.x, p01_index.x), std::min(p10_index.x, p11_index.x));
    int min_y = std::min(std::min(p00_index.y, p01_index.y), std::min(p10_index.y, p11_index.y));
    int max_x = std::max(std::max(p00_index.x, p01_index.x), std::max(p10_index.x, p11_index.x));
    int max_y = std::max(std::max(p00_index.y, p01_index.y), std::max(p10_index.y, p11_index.y));

    ImageBounds ret;
    ret.p0.x = glm::u32(std::clamp(min_x, 0, int(mask->cols() - 1)));
    ret.p0.y = glm::u32(std::clamp(min_y, 0, int(mask->rows() - 1)));
    ret.p1.x = glm::u32(std::clamp(max_x, 0, int(mask->cols() - 1)));
    ret.p1.y = glm::u32(std::clamp(max_y, 0, int(mask->rows() - 1)));

    return ret;
}
};  // namespace ShadowMaskEvaluation