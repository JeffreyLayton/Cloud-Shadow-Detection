#pragma once
#include <memory>

#include "types.h"

namespace ImageOperations {
#ifndef IMAGE_OPERATIONS_TEMPLATES
#    define IMAGE_OPERATIONS_TEMPLATES

template<class T>
bool valid(std::shared_ptr<Image<T>> A, size_t i, size_t j) {
    return ((i >= 0) && (i < A->cols())) && ((j >= 0) && (j < A->rows()));
}
template<class T>
T at(std::shared_ptr<Image<T>> A, size_t i, size_t j) {
    return (*A)(A->rows() - 1 - j, i);
}
template<class T>
void set(std::shared_ptr<Image<T>> A, size_t i, size_t j, T v) {
    (*A)(A->rows() - 1 - j, i) = v;
}
template<class T>
glm::vec2 sides(std::shared_ptr<Image<T>> A, float Diagonal) {
    return Diagonal * glm::normalize(glm::vec2(A->cols(), A->rows()));
}
template<class T>
glm::vec3 pos(
    std::shared_ptr<Image<T>> A,
    float Diagonal,
    unsigned int i,
    unsigned int j,
    float alpha,
    float beta
) {
    glm::vec2 side_lengths = sides<T>(A, Diagonal);
    return {
        side_lengths.x * (float(i) + alpha) / float(A->cols()),
        side_lengths.y * (float(j) + beta) / float(A->rows()),
        0.f};
}
template<class T>
glm::vec3 pos(std::shared_ptr<Image<T>> A, float Diagonal, unsigned int i, unsigned int j) {
    return pos<T>(A, Diagonal, i, j, .5f, .5f);
}
template<class T>
glm::ivec2 index(std::shared_ptr<Image<T>> A, float Diagonal, glm::vec2 pos) {
    glm::vec2 side_lengths = sides<T>(A, Diagonal);
    return {
        floorf(float(A->cols()) * pos.x / side_lengths.x),
        floorf(float(A->rows()) * pos.y / side_lengths.y)};
}

template<class T, class Y>
bool DIM_CHECK(std::shared_ptr<Image<T>> A, std::shared_ptr<Image<Y>> B) {
    return (A->rows() == B->rows()) && (A->cols() == B->cols());
}
template<class ReturnType, class SourceType>
std::shared_ptr<Image<ReturnType>> cast(std::shared_ptr<Image<SourceType>> A) {
    return std::make_shared<Image<ReturnType>>(A->template cast<ReturnType>());
}
template<class T>
std::shared_ptr<Image<T>> cast(std::shared_ptr<ImageBool> A, T true_value, T false_value) {
    std::shared_ptr<Image<T>> ret = std::make_shared<Image<T>>(A->rows(), A->cols());
    for (int i = 0; i < A->size(); i++)
        ret->data()[i] = A->data()[i] ? true_value : false_value;
    return ret;
}
template<class T>
std::shared_ptr<Image<T>> clone(std::shared_ptr<Image<T>> A) {
    return cast<T, T>(A);
}
template<class T>
std::shared_ptr<Image<T>>
obscure(std::shared_ptr<Image<T>> A, std::shared_ptr<ImageBool> Mask, T replace) {
    if (!DIM_CHECK<T, bool>(A, Mask)) return nullptr;
    std::shared_ptr<Image<T>> ret = std::make_shared<Image<T>>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        ret->data()[i] = Mask->data()[i] ? replace : A->data()[i];
    return ret;
}
template<class T>
std::shared_ptr<std::pair<std::vector<T>, std::vector<T>>>
partitionUnobscuredObscured(std::shared_ptr<Image<T>> A, std::shared_ptr<ImageBool> Mask) {
    if (!DIM_CHECK<T, bool>(A, Mask)) return nullptr;
    std::shared_ptr<std::pair<std::vector<T>, std::vector<T>>> ret
        = std::make_shared<std::pair<std::vector<T>, std::vector<T>>>();
    for (int i = 0; i < A->size(); i++) {
        if (!Mask->data()[i]) ret->second.push_back(A->data()[i]);
        else ret->first.push_back(A->data()[i]);
    }
    return ret;
}
#endif  // !IMAGE_OPERATIONS_TEMPLATES
std::shared_ptr<ImageBool> Threshold(std::shared_ptr<ImageFloat> A, float threshold);
std::shared_ptr<ImageBool> Threshold(std::shared_ptr<ImageInt> A, int threshold);
std::shared_ptr<ImageBool> Threshold(std::shared_ptr<ImageUint> A, unsigned int threshold);

std::shared_ptr<ImageBool> NOT(std::shared_ptr<ImageBool> A);
std::shared_ptr<ImageBool> AND(std::shared_ptr<ImageBool> A, std::shared_ptr<ImageBool> B);
std::shared_ptr<ImageBool> OR(std::shared_ptr<ImageBool> A, std::shared_ptr<ImageBool> B);
std::vector<glm::uvec2>
flood(std::shared_ptr<ImageBool> A, unsigned int i_start, unsigned int j_start);

std::shared_ptr<ImageFloat> MIN(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B);
std::shared_ptr<ImageFloat> MAX(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B);
std::shared_ptr<ImageFloat> ADD(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B);
std::shared_ptr<ImageFloat> SUBTRACT(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B);
std::shared_ptr<ImageFloat> DIVIDE(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B);
std::shared_ptr<ImageFloat> NEGATE(std::shared_ptr<ImageFloat> A);

std::shared_ptr<ImageUint> ADD(std::shared_ptr<ImageUint> A, std::shared_ptr<ImageUint> B);

std::shared_ptr<ImageFloat> normalize(
    std::shared_ptr<ImageUint> A,
    unsigned int max = std::numeric_limits<unsigned int>::max()
);
std::shared_ptr<ImageFloat>
normalize(std::shared_ptr<ImageInt> A, int max = std::numeric_limits<int>::max());
std::shared_ptr<ImageFloat>
normalize(std::shared_ptr<ImageFloat> A, float max = std::numeric_limits<float>::max());
std::shared_ptr<ImageFloat> toDegrees(std::shared_ptr<ImageFloat> A);
std::shared_ptr<ImageFloat> toRadians(std::shared_ptr<ImageFloat> A);

std::vector<float> decomposeRBGA(std::shared_ptr<ImageUint> A);
std::vector<uint8_t> decomposeRBGA256(std::shared_ptr<ImageUint> A);
unsigned int CoverCount(std::shared_ptr<ImageBool> A);
float CoverPercentage(std::shared_ptr<ImageBool> A);

unsigned int SubCoverCount(std::shared_ptr<ImageBool> A, ImageBounds bounds);
}  // namespace ImageOperations
