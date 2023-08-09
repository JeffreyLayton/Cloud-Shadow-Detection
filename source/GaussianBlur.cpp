#include "GaussianBlur.h"

#include "ComputeEnvironment.h"
#include "Functions.h"

#define _USE_MATH_DEFINES
#include <boost/compute/container/vector.hpp>
#include <boost/compute/core.hpp>
#include <boost/compute/utility/source.hpp>
#include <math.h>

#include "boilerplate/Log.h"

using namespace boost::compute;
using namespace ComputeEnvironment;
using namespace Functions;

namespace GaussianBlur {
program Program;
kernel KernelVertical;
kernel KernelHorizontal;
vector<float> image1;
vector<float> kernel_strip;
vector<float> image2;

const char cl_kernal_code[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
    int reflect(int v, int end) { return (v < 0)   ? -v
                                      : (v >= end) ? 2 * end - v - 1
                                                   : v; }

    __kernel void Gaussian1DVertical(
        __global const float *inputImage,
        const int width,
        const int height,
        __global const float *kernel_buff,
        const int kernelRadius,
        __global float *outputImage
    ) {
        int x = get_global_id(0);
        int y = get_global_id(1);
        if (x < 0 || x >= width || y < 0 || y >= height) return;

        int index = x + y * width;
        float out = kernel_buff[0] * inputImage[index];
        for (int i = 1; i <= kernelRadius; i++) {
            int yp     = reflect(y + i, height);
            int yn     = reflect(y - i, height);
            int indexp = x + yp * width;
            int indexn = x + yn * width;
            out += kernel_buff[i] * (inputImage[indexp] + inputImage[indexn]);
        }
        outputImage[index] = out;
    }

    __kernel void Gaussian1DHorizontal(
        __global const float *inputImage,
        const int width,
        const int height,
        __global const float *kernel_buff,
        const int kernelRadius,
        __global float *outputImage
    ) {
        int x = get_global_id(0);
        int y = get_global_id(1);
        if (x < 0 || x >= width || y < 0 || y >= height) return;

        int index = x + y * width;
        float out = kernel_buff[0] * inputImage[index];
        for (int i = 1; i <= kernelRadius; i++) {
            int xp     = reflect(x + i, width);
            int xn     = reflect(x - i, width);
            int indexp = xp + y * width;
            int indexn = xn + y * width;
            out += kernel_buff[i] * (inputImage[indexp] + inputImage[indexn]);
        }
        outputImage[index] = out;
    }

);

void init() {
    try {
        Program = program::create_with_source(cl_kernal_code, ComputeEnvironment::Context);
        Program.build();
        KernelVertical   = kernel(Program, "Gaussian1DVertical");
        KernelHorizontal = kernel(Program, "Gaussian1DHorizontal");
        image1           = vector<float>(1, Context);
        image2           = vector<float>(1, Context);
    } catch (opencl_error error) {
        Log::error("OpenCL Error: {} returned {}", error.what(), error.error_string());
    }
}

std::vector<float> StripKernel(float sigma) {
    std::vector<float> kernel_cpu(size_t(2.f * sigma) + 1);
    float norm   = 1.f / (sqrtf(2.f * float(M_PI)) * sigma);
    float Rcoeff = 1.f / (2.f * sigma * sigma);
    float total  = 0.f;
    for (int i = 0; i < kernel_cpu.size(); i++) {
        float v       = norm * expf(-float(i) * float(i) * Rcoeff);
        kernel_cpu[i] = (sigma > 1.e-6f) ? v : float(i == 0);
        total += (i > 0) ? 2.f * kernel_cpu[i] : kernel_cpu[i];
    }
    total = 1.f / total;
    for (int i = 0; i < kernel_cpu.size(); i++)
        kernel_cpu[i] *= total;
    return kernel_cpu;
}

std::shared_ptr<ImageFloat> GaussianBlurFilter(std::shared_ptr<ImageFloat> in, float sigma) {
    // Size the data properly and/or upload
    if (image1.size() != in->size()) {
        image1 = vector<float>(in->size(), Context);
        image2 = vector<float>(in->size(), Context);
    }
    copy(in->data(), in->data() + in->size(), image1.begin(), CommandQueue);

    // Generate our kernel (1D)
    std::vector<float> kernel_cpu = StripKernel(sigma);
    if (kernel_strip.size() != kernel_cpu.size())
        kernel_strip = vector<float>(kernel_cpu.size(), Context);
    copy(kernel_cpu.begin(), kernel_cpu.end(), kernel_strip.begin(), CommandQueue);

    // Define our compute sizes
    const size_t global_work_size[2]
        = {ceilingMultiple<size_t>(in->cols(), 8), ceilingMultiple<size_t>(in->rows(), 8)};
    const size_t local_work_size[2] = {8, 8};

    // Horizontal
    try {
        KernelHorizontal.set_arg(0, image1.get_buffer());
        KernelHorizontal.set_arg(1, int(in->cols()));
        KernelHorizontal.set_arg(2, int(in->rows()));
        KernelHorizontal.set_arg(3, kernel_strip.get_buffer());
        KernelHorizontal.set_arg(4, int(kernel_cpu.size()) - 1);
        KernelHorizontal.set_arg(5, image2.get_buffer());
        CommandQueue.enqueue_nd_range_kernel(
            KernelHorizontal, 2, 0, global_work_size, local_work_size
        );
        CommandQueue.finish();
    } catch (opencl_error error) {
        Log::error("OpenCL Error: {} returned {}", error.what(), error.error_string());
    }

    // Vertical
    try {
        KernelVertical.set_arg(0, image2.get_buffer());
        KernelVertical.set_arg(1, int(in->cols()));
        KernelVertical.set_arg(2, int(in->rows()));
        KernelVertical.set_arg(3, kernel_strip.get_buffer());
        KernelVertical.set_arg(4, int(kernel_cpu.size()) - 1);
        KernelVertical.set_arg(5, image1.get_buffer());
        CommandQueue.enqueue_nd_range_kernel(
            KernelVertical, 2, 0, global_work_size, local_work_size
        );
        CommandQueue.finish();
    } catch (opencl_error error) {
        Log::error("OpenCL Error: {} returned {}", error.what(), error.error_string());
    }

    // Return value
    std::shared_ptr<ImageFloat> ret = std::make_shared<ImageFloat>(in->rows(), in->cols());
    copy(image1.begin(), image1.end(), ret->data(), CommandQueue);
    return ret;
}
}  // namespace GaussianBlur