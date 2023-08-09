#include "SceneClassificationLayer.h"

std::shared_ptr<ImageBool>
SceneClassificationLayer::GenerateMask(std::shared_ptr<ImageUint> A, unsigned int channelCodes) {
    std::shared_ptr<ImageBool> ret = std::make_shared<ImageBool>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        switch (A->data()[i]) {
            case NO_DATA_VALUE: ret->data()[i] = (channelCodes & NO_DATA_MASK) > 0; break;
            case SATURATED_DEFECTIVE_VALUE:
                ret->data()[i] = (channelCodes & SATURATED_DEFECTIVE_MASK) > 0;
                break;
            case DARK_AREA_PIXELS_VALUE:
                ret->data()[i] = (channelCodes & DARK_AREA_PIXELS_MASK) > 0;
                break;
            case CLOUD_SHADOWS_VALUE:
                ret->data()[i] = (channelCodes & CLOUD_SHADOWS_MASK) > 0;
                break;
            case VEGITATION_VALUE: ret->data()[i] = (channelCodes & VEGITATION_MASK) > 0; break;
            case BARE_SOIL_VALUE: ret->data()[i] = (channelCodes & BARE_SOIL_MASK) > 0; break;
            case WATER_VALUE: ret->data()[i] = (channelCodes & WATER_MASK) > 0; break;
            case CLOUD_LOW_VALUE: ret->data()[i] = (channelCodes & CLOUD_LOW_MASK) > 0; break;
            case CLOUD_MEDIUM_VALUE: ret->data()[i] = (channelCodes & CLOUD_MEDIUM_MASK) > 0; break;
            case CLOUD_HIGH_VALUE: ret->data()[i] = (channelCodes & CLOUD_HIGH_MASK) > 0; break;
            case CLOUD_CIRRUS_VALUE: ret->data()[i] = (channelCodes & CLOUD_CIRRUS_MASK) > 0; break;
            case SNOW_ICE_VALUE: ret->data()[i] = (channelCodes & SNOW_ICE_MASK) > 0; break;
            default: ret->data()[i] = false; break;
        }
    return ret;
}

std::shared_ptr<ImageUint> SceneClassificationLayer::GenerateRGBA(std::shared_ptr<ImageUint> A) {
    std::shared_ptr<ImageUint> ret = std::make_shared<ImageUint>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        switch (A->data()[i]) {
            case SATURATED_DEFECTIVE_VALUE: ret->data()[i] = SATURATED_DEFECTIVE_COLOUR; break;
            case DARK_AREA_PIXELS_VALUE: ret->data()[i] = DARK_AREA_PIXELS_COLOUR; break;
            case CLOUD_SHADOWS_VALUE: ret->data()[i] = CLOUD_SHADOWS_COLOUR; break;
            case VEGITATION_VALUE: ret->data()[i] = VEGITATION_COLOUR; break;
            case BARE_SOIL_VALUE: ret->data()[i] = BARE_SOIL_COLOUR; break;
            case WATER_VALUE: ret->data()[i] = WATER_COLOUR; break;
            case CLOUD_LOW_VALUE: ret->data()[i] = CLOUD_LOW_COLOUR; break;
            case CLOUD_MEDIUM_VALUE: ret->data()[i] = CLOUD_MEDIUM_COLOUR; break;
            case CLOUD_HIGH_VALUE: ret->data()[i] = CLOUD_HIGH_COLOUR; break;
            case CLOUD_CIRRUS_VALUE: ret->data()[i] = CLOUD_CIRRUS_COLOUR; break;
            case SNOW_ICE_VALUE: ret->data()[i] = SNOW_ICE_COLOUR; break;
            default: ret->data()[i] = NO_DATA_COLOUR; break;
        }
    return ret;
}