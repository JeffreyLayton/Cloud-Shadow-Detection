#include "ProbabilityRefinement.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <list>
#include <map>

#include "Functions.h"
#include "ImageOperations.h"

namespace ImOp = ImageOperations;
using namespace Functions;

float __f__(float x, float a, float b) { return 1.0 / (1.0 + b * exp(-a * x)); }

std::shared_ptr<ImageFloat> ProbabilityRefinement::AlphaMap(
    std::shared_ptr<ImageFloat> NIR_difference
) {
    // Setup
    float a = 17.f, b = .007f;
#define f(x) __f__(x, a, b)
    float sub   = f(-.5f);
    float denom = 1.f;
#define F(x) (f(x - .5f) - sub) * denom;
    std::shared_ptr<ImageFloat> ret
        = std::make_shared<ImageFloat>(NIR_difference->rows(), NIR_difference->cols());
    for (int i = 0; i < ret->size(); i++)
        ret->data()[i] = F(NIR_difference->data()[i]);
    return ret;
}

std::shared_ptr<ImageFloat> ProbabilityRefinement::BetaMap(
    ShadowQuads shadows,
    std::map<int, CloudShadowMatching::OptimalSolution> solutions,
    std::shared_ptr<ImageBool> cloudMask,
    std::shared_ptr<ImageBool> shadowMask,
    std::shared_ptr<ImageFloat> CLP,
    float DiagonalLength
) {
    static const float min_distance           = 5.f;
    static const float max_distance           = 80.f;
    static const float mid_percentile         = .2f;
    static const float min_factor             = .15f;
    static const float area_correction_factor = 2.f * M_2_SQRTPI;

    std::shared_ptr<ImageBool> map  = std::make_shared<ImageBool>(CLP->rows(), CLP->cols());
    std::shared_ptr<ImageFloat> ret = std::make_shared<ImageFloat>(CLP->rows(), CLP->cols());
    ret->fill(0.f);

    for (auto &s : shadows) {
        glm::mat4 M_inverse = glm::inverse(solutions[s.first].M);
        // Apply distance to factor function and generate function area
        float influence_distance_f = std::clamp(
            area_correction_factor * sqrtf(float(s.second.pixels.list.size())),
            min_distance,
            max_distance
        );
        int influence_distance_i = int(floorf(influence_distance_f));
        ImageBounds influence_bounds
            = {{(unsigned int)(std::clamp(
                    int(s.second.pixels.bounds.p0.x) - influence_distance_i, 0, int(CLP->cols()) - 1
                )),
                (unsigned int)(std::clamp(
                    int(s.second.pixels.bounds.p0.y) - influence_distance_i, 0, int(CLP->rows()) - 1
                ))},
               {(unsigned int)(std::clamp(
                    int(s.second.pixels.bounds.p1.x) + influence_distance_i, 0, int(CLP->cols()) - 1
                )),
                (unsigned int)(std::clamp(
                    int(s.second.pixels.bounds.p1.y) + influence_distance_i, 0, int(CLP->rows()) - 1
                ))}};
        // We onluy need to check borders for distance
        Shadow shadow_border = border(s.second.pixels);
        // Reset map for shadow;
        map->fill(false);
        for (auto &pix : s.second.pixels.list)
            ImOp::set(map, pix.x, pix.y, true);
        // For each pixel in bounds
        for (unsigned int i = influence_bounds.p0.x; i <= influence_bounds.p1.x; i++) {
            for (unsigned int j = influence_bounds.p0.y; j <= influence_bounds.p1.y; j++) {
                // Set as max distance
                float current_distance = max<float>();
                // Not a shadow pixel
                if (!ImOp::at(map, i, j))
                    for (auto &p : shadow_border.list)  // Find Closest Pixel
                        current_distance = glm::min(current_distance, pixelDistance(p, {i, j}));
                else current_distance = 0.f;  // No distance since it is a shadow Pixel
                // If the closest pixel is close enough
                if (current_distance <= influence_distance_f) {
                    // Calculate the distance factor
                    float factor = quadraticRadialBasis(
                        current_distance,
                        influence_distance_f * min_factor,
                        influence_distance_f,
                        mid_percentile
                    );
                    // Find the corresponding cloud pixel
                    glm::ivec2 cloud_space_index = ImOp::index(
                        cloudMask,
                        DiagonalLength,
                        glm::vec2(
                            M_inverse * glm::vec4(ImOp::pos(shadowMask, DiagonalLength, i, j), 1.f)
                        )
                    );
                    // If there exists a valid cloud pixel
                    if (ImOp::valid(CLP, cloud_space_index.x, cloud_space_index.y)) {
                        // Obtain the
                        float CLP_v = ImOp::at(CLP, cloud_space_index.x, cloud_space_index.y);
                        ImOp::set(ret, i, j, std::max(CLP_v * factor, ImOp::at(ret, i, j)));
                    }
                }
            }
        }
    }
    return ret;
}

ProbabilityRefinement::UniformProbabilitySurface __Sample_Surface__(
    std::shared_ptr<ImageBool> mask,
    std::shared_ptr<ImageFloat> alphaMap,
    std::shared_ptr<ImageFloat> betaMap,
    unsigned int div
) {
    std::shared_ptr<ImageUint> total  = std::make_shared<ImageUint>(div, div);
    std::shared_ptr<ImageUint> shadow = std::make_shared<ImageUint>(div, div);
    total->fill(0u);
    shadow->fill(0u);
    for (int i = 0; i < alphaMap->size(); i++) {
        int cellx = std::min(int(floorf(alphaMap->data()[i] * float(div))), int(div) - 1);
        int celly = std::min(int(floorf(betaMap->data()[i] * float(div))), int(div) - 1);
        ImOp::set(total, cellx, celly, ImOp::at(total, cellx, celly) + 1u);
        if (mask->data()[i]) ImOp::set(shadow, cellx, celly, ImOp::at(shadow, cellx, celly) + 1u);
    }

    auto result = ImOp::DIVIDE(ImOp::cast<float>(shadow), ImOp::cast<float>(total));

    ProbabilityRefinement::UniformProbabilitySurface ret({div, div});
    for (int i = 0; i < div; i++) {
        for (int j = 0; j < div; j++) {
            if (ImOp::at(total, i, j) > 0u) ret.set(i, j, ImOp::at(result, i, j));
        }
    }
}

ProbabilityRefinement::UniformProbabilitySurface
__ProbabilityMap__Element(std::vector<glm::vec3> &samples, unsigned int D) {
    std::vector<std::pair<glm::vec3, unsigned int>> collections(D * D);
    for (auto &s : samples) {
        int i                         = std::max(std::min(int(floorf(s.x * D)), int(D) - 1), 0);
        int j                         = std::max(std::min(int(floorf(s.y * D)), int(D) - 1), 0);
        collections[i + D * j].first  = collections[i + D * j].first + s;
        collections[i + D * j].second = collections[i + D * j].second + 1;
    }

    ProbabilityRefinement::UniformProbabilitySurface ret({D, D});
#define VALUE(i, j) collections[i + D * j].first
#define COUNT(i, j) collections[i + D * j].second
#define VALID(i, j) COUNT(i, j) > 0
    std::list<glm::ivec2> empty_pixels;
    for (int i = 0; i < D; i++)
        for (int j = 0; j < D; j++)
            if (VALID(i, j)) ret.set(i, j, (VALUE(i, j) / float(COUNT(i, j))).z);
            else empty_pixels.push_back({i, j});

#define VALID_N(i, j) (i < 0 || i >= D || j < 0 || j >= D) ? false : VALID(i, j)
#define VALUE_N(i, j) (i < 0 || i >= D || j < 0 || j >= D) ? 0.f : ret.at(i, j)
    while (!empty_pixels.empty()) {
        for (auto it = empty_pixels.begin(); it != empty_pixels.end(); it++) {
            auto &v           = *it;
            float accum       = 0.f;
            float totalWeight = 0.f;
            bool calc         = false;
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (VALID_N(v.x + i, v.y + j)) {
                        calc = true;
                        accum += ret.at(v.x + i, v.y + j) / float(i * i + j * j);
                        totalWeight += 1.f / float(i * i + j * j);
                    }
                }
            }
            if (calc) {
                COUNT(v.x, v.y) = 1;
                ret.set(v.x, v.y, accum / totalWeight);
                empty_pixels.erase(it--);
            }
        }
    }

    return ret;
}

ProbabilityRefinement::UniformProbabilitySurface ProbabilityRefinement::ProbabilityMap(
    std::shared_ptr<ImageBool> shadowMask,
    std::shared_ptr<ImageFloat> alphaMap,
    std::shared_ptr<ImageFloat> betaMap
) {
    static const unsigned int D[] = {8u, 16u, 32u, 64u, 128u};
    static const float W[]        = {16.f / 31.f, 8.f / 31.f, 4.f / 31.f, 2.f / 31.f, 1.f / 31.f};

    std::vector<glm::vec3> samples;
    for (int i = 0; i < shadowMask->size(); i++)
        samples.push_back(
            {alphaMap->data()[i], betaMap->data()[i], shadowMask->data()[i] ? 1.f : 0.f}
        );

    UniformProbabilitySurface elements[5];

    for (int i = 0; i < 5; i++)
        elements[i] = __ProbabilityMap__Element(samples, D[i]);

    UniformProbabilitySurface ret({256, 256});
    ret.set(Bounds::ALPHA_MIN, 0.f);
    ret.set(Bounds::BETA_MIN, 0.f);
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            if (i == 0) ret.set(i, j, 0.f);
            else {
                float v = 0.f;
                float a = (i + .5f) / 256;
                float b = (j + .5f) / 256;
                for (int i = 0; i < 5; i++)
                    v += W[i] * elements[i](a, b);
                ret.set(i, j, std::max(std::min(v, 1.f), 0.f));
            }
        }
    }
    return ret;
}

std::shared_ptr<ImageBool> ProbabilityRefinement::ImprovedShadowMask(
    std::shared_ptr<ImageBool> shadowMask,
    std::shared_ptr<ImageBool> cloudMask,
    std::shared_ptr<ImageFloat> alphaMap,
    std::shared_ptr<ImageFloat> betaMap,
    UniformProbabilitySurface probabilitySurface,
    float threshold
) {
    std::shared_ptr<ImageBool> ret
        = std::make_shared<ImageBool>(shadowMask->rows(), shadowMask->cols());
    ret->fill(false);
    for (int i = 0; i < shadowMask->cols(); i++)
        for (int j = 0; j < shadowMask->rows(); j++)
            if (threshold <= probabilitySurface(ImOp::at(alphaMap, i, j), ImOp::at(betaMap, i, j)))
                ImOp::set(ret, i, j, true);
    return ImOp::AND(ImOp::OR(ret, shadowMask), ImOp::NOT(cloudMask));
}

ProbabilityRefinement::UniformProbabilitySurface::UniformProbabilitySurface() {}

ProbabilityRefinement::UniformProbabilitySurface ProbabilityRefinement::testMap() {
    UniformProbabilitySurface ret({64, 64});
    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 64; j++)
            ret.set(i, j, std::min(i / 64.f, j / 64.f));
    ret.set(Bounds::BETA_MIN, 0.f);
    ret.set(Bounds::BETA_MAX, 1.f);
    ret.set(Bounds::ALPHA_MIN, 0.f);
    ret.set(Bounds::ALPHA_MAX, 1.f);
    return ret;
}

ProbabilityRefinement::UniformProbabilitySurface::UniformProbabilitySurface(glm::uvec2 divs)
    : m_data(std::make_shared<ImageFloat>(divs.x, divs.y)) {
    m_data->fill(0.f);
}

float ProbabilityRefinement::UniformProbabilitySurface::operator()(float alpha, float beta) {
    float cellx = alpha * float(m_data->cols());
    float celly = beta * float(m_data->rows());

    int x_max = int(roundf(cellx));
    int y_max = int(roundf(celly));
    int x_min = x_max - 1;
    int y_min = y_max - 1;

    float P0 = this->at(x_min, y_min);  // P2 --- P3
    float P1 = this->at(x_max, y_min);  //  |     |
    float P2 = this->at(x_min, y_max);  //  |     |
    float P3 = this->at(x_max, y_max);  // P0 --- P1

    float u = cellx - (float(x_min) + .5f);
    float v = celly - (float(y_min) + .5f);

    return bilinear(P0, P1, P2, P3, u, v);
}

float ProbabilityRefinement::UniformProbabilitySurface::dAlpha() {
    return 1.f / float(m_data->cols());
}

float ProbabilityRefinement::UniformProbabilitySurface::dBeta() {
    return 1.f / float(m_data->rows());
}

float ProbabilityRefinement::UniformProbabilitySurface::at(int i, int j) {
    bool left     = i < 0;
    bool right    = i >= m_data->cols();
    bool down     = j < 0;
    bool up       = j >= m_data->rows();
    bool middle_x = !(left || right);
    bool middle_y = !(up || down);
    /*
               |         |
     L & U |    U    | R & U
               |         |
    <----- X ------- X ----->
               |* * * * *|
       L   |* Exact *|   R
               |* * * * *|
    <----- X ------- X ----->
               |         |
     L & D |    D	 | R & D
               |         |
    */

    // Exact element *Normally
    if (middle_x && middle_y) [[likely]]
        return ImOp::at(m_data, i, j);
    //
    // Single Interpolation
    else if (left && middle_y) {  // i < 0
        if (m_alpha_min_clamp.has_value())
            return linear(m_alpha_min_clamp.value(), ImOp::at(m_data, 0, j), float(2 * i + 1));
        return linear(ImOp::at(m_data, 0, j), ImOp::at(m_data, 1, j), float(i));
    } else if (right && middle_y) {  // i >= width
        if (m_alpha_max_clamp.has_value())
            return linear(
                ImOp::at(m_data, m_data->cols() - 1, j),
                m_alpha_max_clamp.value(),
                float(2 * (i + 1 - m_data->cols()))
            );
        return linear(
            ImOp::at(m_data, m_data->cols() - 2, j),
            ImOp::at(m_data, m_data->cols() - 1, j),
            float(i + 2 - m_data->cols())
        );
    } else if (middle_x && down) {  // j < 0
        if (m_beta_min_clamp.has_value())
            return linear(m_beta_min_clamp.value(), ImOp::at(m_data, i, 0), float(2 * j + 1));
        return linear(ImOp::at(m_data, i, 0), ImOp::at(m_data, i, 1), float(j));
    } else if (middle_x && up) {  // j >= height
        if (m_beta_max_clamp.has_value())
            return linear(
                ImOp::at(m_data, i, m_data->rows() - 1),
                m_beta_max_clamp.value(),
                float(2 * (j + 1 - m_data->rows()))
            );
        return linear(
            ImOp::at(m_data, i, m_data->rows() - 2),
            ImOp::at(m_data, i, m_data->rows() - 1),
            float(j + 2 - m_data->rows())
        );
    }
    // Double Interpolation
    else if (left && down) {  // i < 0      and j < 0
        float d_to_x_axis = float(-j);
        float d_to_y_axis = float(-i);
        return linear(this->at(i, 0), this->at(0, j), d_to_x_axis / (d_to_x_axis + d_to_y_axis));
    } else if (right && down) {  // i >= width and j < 0
        float d_to_x_axis = float(-j);
        float d_to_y_axis = float(i + 1 - m_data->cols());
        return linear(
            this->at(i, 0),
            this->at(m_data->cols() - 1, j),
            d_to_x_axis / (d_to_x_axis + d_to_y_axis)
        );
    } else if (left && up) {  // i < 0      and j >= height
        float d_to_x_axis = float(j + 1 - m_data->rows());
        float d_to_y_axis = float(-i);
        return linear(
            this->at(i, m_data->rows() - 1),
            this->at(0, j),
            d_to_x_axis / (d_to_x_axis + d_to_y_axis)
        );
    } else if (right && up) {  // i >= width and j >= height
        float d_to_x_axis = float(j + 1 - m_data->rows());
        float d_to_y_axis = float(i + 1 - m_data->cols());
        return linear(
            this->at(i, m_data->rows() - 1),
            this->at(m_data->cols() - 1, j),
            d_to_x_axis / (d_to_x_axis + d_to_y_axis)
        );
    }
    return 0.f;
}

void ProbabilityRefinement::UniformProbabilitySurface::set(int i, int j, float v) {
    ImOp::set(m_data, i, j, v);
}

void ProbabilityRefinement::UniformProbabilitySurface::set(Bounds axis, float v) {
    switch (axis) {
        case Bounds::ALPHA_MIN: m_alpha_min_clamp = v; break;
        case Bounds::ALPHA_MAX: m_alpha_max_clamp = v; break;
        case Bounds::BETA_MIN: m_beta_min_clamp = v; break;
        case Bounds::BETA_MAX: m_beta_max_clamp = v; break;
    }
}

void ProbabilityRefinement::UniformProbabilitySurface::clear(Bounds axis) {
    switch (axis) {
        case Bounds::ALPHA_MIN: m_alpha_min_clamp.reset(); break;
        case Bounds::ALPHA_MAX: m_alpha_max_clamp.reset(); break;
        case Bounds::BETA_MIN: m_beta_min_clamp.reset(); break;
        case Bounds::BETA_MAX: m_beta_max_clamp.reset(); break;
    }
}

glm::uvec2 ProbabilityRefinement::UniformProbabilitySurface::resolution() {
    return {m_data->cols(), m_data->rows()};
}

ProbabilityRefinement::SurfaceRenderGeom ProbabilityRefinement::UniformProbabilitySurface::MeshData(
    int i_min,
    int i_max,
    int j_min,
    int j_max
) {
    SurfaceRenderGeom ret;
    for (int i = i_min; i <= i_max; i++)
        for (int j = j_min; j <= j_max; j++) {
            ret.verts.push_back({(2 * i + 1) * dAlpha() - 1, (2 * j + 1) * dBeta() - 1, at(i, j)});
            if (i < i_max && j < j_max) {
                ret.tris.push_back({i, j, 0});
                ret.tris.push_back({i, j, 1});
            }
        }
    return ret;
}