#include <queue>

#include "ImageOperations.h"

namespace ImageOperations {
std::shared_ptr<ImageBool> Threshold(std::shared_ptr<ImageFloat> A, float threshold) {
    std::shared_ptr<ImageBool> ret = std::make_shared<ImageBool>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        ret->data()[i] = A->data()[i] >= threshold;
    return ret;
}
std::shared_ptr<ImageBool> Threshold(std::shared_ptr<ImageInt> A, int threshold) {
    std::shared_ptr<ImageBool> ret = std::make_shared<ImageBool>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        ret->data()[i] = A->data()[i] >= threshold;
    return ret;
}
std::shared_ptr<ImageBool> Threshold(std::shared_ptr<ImageUint> A, unsigned int threshold) {
    std::shared_ptr<ImageBool> ret = std::make_shared<ImageBool>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        ret->data()[i] = A->data()[i] >= threshold;
    return ret;
}

std::shared_ptr<ImageBool> NOT(std::shared_ptr<ImageBool> A) {
    std::shared_ptr<ImageBool> ret = std::make_shared<ImageBool>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        ret->data()[i] = !A->data()[i];
    return ret;
}
std::shared_ptr<ImageBool> AND(std::shared_ptr<ImageBool> A, std::shared_ptr<ImageBool> B) {
    if (!DIM_CHECK(A, B)) return nullptr;
    return std::make_shared<ImageBool>(A->array().min(B->array()));
}
std::shared_ptr<ImageBool> OR(std::shared_ptr<ImageBool> A, std::shared_ptr<ImageBool> B) {
    if (!DIM_CHECK(A, B)) return nullptr;
    return std::make_shared<ImageBool>(A->array().max(B->array()));
}
std::vector<glm::uvec2>
flood(std::shared_ptr<ImageBool> A, unsigned int i_start, unsigned int j_start) {
    std::queue<glm::uvec2> queue;
    queue.push(glm::uvec2(i_start, j_start));
    glm::uvec2 current;
    std::vector<glm::uvec2> pixelList;
    std::shared_ptr<ImageBool> used = std::make_shared<ImageBool>(A->rows(), A->cols());
    used->fill(false);
    set(used, i_start, j_start, true);
    do {
        current = queue.front();
        queue.pop();
        if (at(A, current.x, current.y)) {
            pixelList.push_back(current);
            for (int i = std::max(0, int(current.x) - 1);
                 i < std::min(int(A->cols()), int(current.x) + 2);
                 i++) {
                for (int j = std::max(0, int(current.y) - 1);
                     j < std::min(int(A->rows()), int(current.y) + 2);
                     j++) {
                    if (!at(used, i, j)) {  // Not already concidered
                        queue.push(glm::uvec2(i, j));
                        set(used, i, j, true);
                    }
                }
            }
        }
    } while (!queue.empty());
    return pixelList;
}

std::shared_ptr<ImageFloat> MIN(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B) {
    if (!DIM_CHECK(A, B)) return nullptr;
    return std::make_shared<ImageFloat>(A->array().min(B->array()));
}
std::shared_ptr<ImageFloat> MAX(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B) {
    if (!DIM_CHECK(A, B)) return nullptr;
    return std::make_shared<ImageFloat>(A->array().max(B->array()));
}
std::shared_ptr<ImageFloat> ADD(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B) {
    if (!DIM_CHECK(A, B)) return nullptr;
    return std::make_shared<ImageFloat>((*A) + (*B));
}
std::shared_ptr<ImageFloat> SUBTRACT(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B) {
    if (!DIM_CHECK(A, B)) return nullptr;
    return std::make_shared<ImageFloat>((*A) - (*B));
}
std::shared_ptr<ImageFloat> DIVIDE(std::shared_ptr<ImageFloat> A, std::shared_ptr<ImageFloat> B) {
    if (!DIM_CHECK(A, B)) return nullptr;
    return std::make_shared<ImageFloat>((*A).cwiseQuotient(*B));
}
std::shared_ptr<ImageFloat> NEGATE(std::shared_ptr<ImageFloat> A) {
    return std::make_shared<ImageFloat>(-(*A));
}

std::shared_ptr<ImageUint> ADD(std::shared_ptr<ImageUint> A, std::shared_ptr<ImageUint> B) {
    if (!DIM_CHECK(A, B)) return nullptr;
    return std::make_shared<ImageUint>((*A) + (*B));
}

std::shared_ptr<ImageFloat> normalize(std::shared_ptr<ImageUint> A, unsigned int max) {
    return std::make_shared<ImageFloat>(A->cast<float>() / float(max));
}
std::shared_ptr<ImageFloat> normalize(std::shared_ptr<ImageInt> A, int max) {
    return std::make_shared<ImageFloat>(A->cast<float>() / float(max));
}
std::shared_ptr<ImageFloat> normalize(std::shared_ptr<ImageFloat> A, float max) {
    return std::make_shared<ImageFloat>(*A / max);
}
std::shared_ptr<ImageFloat> toDegrees(std::shared_ptr<ImageFloat> A) {
    std::shared_ptr<ImageFloat> ret = std::make_shared<ImageFloat>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        ret->data()[i] = glm::degrees(A->data()[i]);
    return ret;
}
std::shared_ptr<ImageFloat> toRadians(std::shared_ptr<ImageFloat> A) {
    std::shared_ptr<ImageFloat> ret = std::make_shared<ImageFloat>(A->rows(), A->cols());
    for (int i = 0; i < ret->size(); i++)
        ret->data()[i] = glm::radians(A->data()[i]);
    return ret;
}
std::vector<float> decomposeRBGA(std::shared_ptr<ImageUint> A) {
    std::vector<float> ret(A->size() * 4);
    for (size_t i = 0; i < size_t(A->size()); i++) {
        ret[4 * i + 0]
            = float((A->data()[i] >> 0) & 0xff) / float(std::numeric_limits<uint8_t>::max());
        ret[4 * i + 1]
            = float((A->data()[i] >> 8) & 0xff) / float(std::numeric_limits<uint8_t>::max());
        ret[4 * i + 2]
            = float((A->data()[i] >> 16) & 0xff) / float(std::numeric_limits<uint8_t>::max());
        ret[4 * i + 3]
            = float((A->data()[i] >> 24) & 0xff) / float(std::numeric_limits<uint8_t>::max());
    }
    return ret;
}
std::vector<uint8_t> decomposeRBGA256(std::shared_ptr<ImageUint> A) {
    std::vector<uint8_t> ret(A->size() * 4);
    for (size_t i = 0; i < size_t(A->size()); i++) {
        ret[4 * i + 0] = uint8_t((A->data()[i] >> 0) & 0xff);
        ret[4 * i + 1] = uint8_t((A->data()[i] >> 8) & 0xff);
        ret[4 * i + 2] = uint8_t((A->data()[i] >> 16) & 0xff);
        ret[4 * i + 3] = uint8_t((A->data()[i] >> 24) & 0xff);
    }
    return ret;
}
unsigned int CoverCount(std::shared_ptr<ImageBool> A) { return A->cast<unsigned int>().sum(); }
float CoverPercentage(std::shared_ptr<ImageBool> A) {
    return A->cast<float>().sum() / float(A->size());
}

unsigned int SubCoverCount(std::shared_ptr<ImageBool> A, ImageBounds bounds) {
    unsigned int count = 0u;
    for (size_t i = std::max(0u, bounds.p0.x); i < std::min(glm::u32(A->cols() - 1u), bounds.p1.x);
         i++) {
        for (size_t j = std::max(0u, bounds.p0.y);
             j < std::min(glm::u32(A->rows() - 1u), bounds.p1.y);
             j++) {
            if (at(A, i, j)) count++;
        }
    }
    return count;
}
}  // namespace ImageOperations