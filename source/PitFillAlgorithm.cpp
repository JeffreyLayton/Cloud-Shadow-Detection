#include "PitFillAlgorithm.h"

#include <boost/compute/container/vector.hpp>
#include <boost/compute/core.hpp>
#include <boost/compute/utility/source.hpp>

#include "ComputeEnvironment.h"
#include "Functions.h"
#include "boilerplate/Log.h"

#define _USE_MATH_DEFINES
#include <vector>

#include <math.h>

using namespace boost::compute;
using namespace ComputeEnvironment;
using namespace Functions;

namespace PitFillAlgorithm {
program Program;
kernel Kernel;
vector<float> image1;
vector<float> original;
vector<int> hasChanged;
vector<float> image2;

const char cl_kernal_code[] = BOOST_COMPUTE_STRINGIZE_SOURCE(

    bool equalFloat(float x, float y) { return fabs((float)(x - y)) < 0.0000000001; }

    float getBoundedValue(
        int x,
        int y,
        int width,
        int height,
        float outsideValue,
        __global const float *inputImage
    ) {
        if (x < 0 || x >= width || y < 0 || y >= height) return outsideValue;
        int index = x + y * width;
        return inputImage[index];
    }

    float minNeighbour(
        int x,
        int y,
        int width,
        int height,
        float outsideValue,
        __global const float *inputImage
    ) {
        float v = getBoundedValue(x - 1, y - 1, width, height, outsideValue, inputImage);
        v       = min(getBoundedValue(x + 0, y - 1, width, height, outsideValue, inputImage), v);
        v       = min(getBoundedValue(x + 1, y - 1, width, height, outsideValue, inputImage), v);
        v       = min(getBoundedValue(x - 1, y + 0, width, height, outsideValue, inputImage), v);
        v       = min(getBoundedValue(x + 1, y + 0, width, height, outsideValue, inputImage), v);
        v       = min(getBoundedValue(x - 1, y + 1, width, height, outsideValue, inputImage), v);
        v       = min(getBoundedValue(x + 0, y + 1, width, height, outsideValue, inputImage), v);
        v       = min(getBoundedValue(x + 1, y + 1, width, height, outsideValue, inputImage), v);
        return v;
    }

    __kernel void PitFill(
        __global const float *inputImage,
        const int width,
        const int height,
        __global const float *originalImage,
        const float outsideValue,
        __global int *hasChanged,
        __global float *outputImage
    ) {
        int x = get_global_id(0);
        int y = get_global_id(1);
        if (x < 0 || x >= width || y < 0 || y >= height) return;

        int index = x + y * width;

        float in_v = inputImage[index];
        float or_v = originalImage[index];

        if (!equalFloat(in_v, or_v)) {
            float min_n        = minNeighbour(x, y, width, height, outsideValue, inputImage);
            float ou_v         = max(or_v, min_n);
            outputImage[index] = ou_v;
            if (!equalFloat(in_v, ou_v)) hasChanged[0] = 1;
        } else outputImage[index] = in_v;
    }

);

void init() {
    try {
        Program = program::create_with_source(cl_kernal_code, Context);
        Program.build();
        Kernel     = kernel(Program, "PitFill");
        image1     = vector<float>(1, Context);
        original   = vector<float>(1, Context);
        hasChanged = vector<int>(1, Context);
        image2     = vector<float>(1, Context);
    } catch (opencl_error error) {
        Log::error("OpenCL Error: {} returned {}", error.what(), error.error_string());
    }
}

std::shared_ptr<ImageFloat>
PitFillAlgorithmFilter(std::shared_ptr<ImageFloat> in, float borderValue) {
    std::vector<float> initv(in->size(), 1.f);
    if (image1.size() != in->size()) {
        image1   = vector<float>(in->size(), Context);
        original = vector<float>(in->size(), Context);
        image2   = vector<float>(in->size(), Context);
    }
    copy(in->data(), in->data() + in->size(), original.begin(), CommandQueue);
    copy(initv.data(), initv.data() + initv.size(), image1.begin(), CommandQueue);

    // Define our compute sizes
    // Define our compute sizes
    const size_t global_work_size[2]
        = {ceilingMultiple<size_t>(in->cols(), 8), ceilingMultiple<size_t>(in->rows(), 8)};
    const size_t local_work_size[2] = {8, 8};

    vector<float> *source = &image2;
    vector<float> *destin = &image1;

    std::vector<int> hasChanged_host = {0};

    // int count = 0;
    do {
        hasChanged_host[0] = 0;
        copy(hasChanged_host.begin(), hasChanged_host.end(), hasChanged.begin(), CommandQueue);
        // std::swap(source, destin);
        vector<float> *temp = destin;
        destin              = source;
        source              = temp;
        try {
            Kernel.set_arg(0, source->get_buffer());
            Kernel.set_arg(1, int(in->cols()));
            Kernel.set_arg(2, int(in->rows()));
            Kernel.set_arg(3, original.get_buffer());
            Kernel.set_arg(4, borderValue);
            Kernel.set_arg(5, hasChanged.get_buffer());
            Kernel.set_arg(6, destin->get_buffer());
            CommandQueue.enqueue_nd_range_kernel(Kernel, 2, 0, global_work_size, local_work_size);
            CommandQueue.finish();
        } catch (opencl_error error) {
            Log::error("OpenCL Error: {} returned {}", error.what(), error.error_string());
        }
        copy(hasChanged.begin(), hasChanged.end(), hasChanged_host.begin(), CommandQueue);
        // std::cout << ++count << std::endl;
    } while (hasChanged_host[0]);

    // Return Value
    std::shared_ptr<ImageFloat> ret = std::make_shared<ImageFloat>(in->rows(), in->cols());
    copy(destin->begin(), destin->end(), ret->data(), CommandQueue);
    return ret;
}
}  // namespace PitFillAlgorithm