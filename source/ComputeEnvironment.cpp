#include "ComputeEnvironment.h"

using namespace boost::compute;

namespace ComputeEnvironment {
context Context;
command_queue CommandQueue;

void InitMainContext() {
    auto platforms = system::platforms();
    Context        = context(platforms[0].devices());
    CommandQueue   = command_queue(Context, Context.get_devices()[0]);
}

std::string PlatformAndDeviceInfo() {
    std::stringstream buffer;
    try {
        // Discover number of platforms
        std::vector<platform> platforms = system::platforms();
        buffer << "\nNumber of OpenCL plaforms: " << platforms.size() << std::endl;

        // Investigate each platform
        buffer << "\n-------------------------" << std::endl;
        for (std::vector<platform>::iterator plat = platforms.begin(); plat != platforms.end();
             plat++) {
            buffer << "Platform: " << plat->name() << std::endl;
            buffer << "\tVendor:  " << plat->vendor() << std::endl;
            buffer << "\tVersion: " << plat->version() << std::endl;

            // Discover number of devices
            std::vector<device> devices = plat->devices();
            buffer << "\n\tNumber of devices: " << devices.size() << std::endl;

            // Investigate each device
            for (std::vector<device>::iterator dev = devices.begin(); dev != devices.end(); dev++) {
                buffer << "\t-------------------------" << std::endl;
                buffer << "\t\tName: " << dev->name() << std::endl;
                buffer << "\t\tID: " << dev->id() << std::endl;
                buffer << "\t\tVersion: " << dev->version() << std::endl;

                cl_device_type info = dev->type();
                buffer << "\t\tType: ";
                std::string type;
                if (info & device::gpu) buffer << "GPU  ";
                if (info & device::cpu) buffer << "CPU  ";
                if (info & device::accelerator) buffer << "Accelerator  ";
                buffer << std::endl;

                buffer << "\t\tMax. Compute Units: " << dev->compute_units() << std::endl;
                buffer << "\t\tLocal Memory Size: " << dev->local_memory_size() / 1024 << " KB"
                       << std::endl;
                buffer << "\t\tGlobal Memory Size: " << dev->global_memory_size() / (1024 * 1024)
                       << " MB" << std::endl;
                buffer << "\t\tMax Alloc Size: " << dev->max_memory_alloc_size() / (1024 * 1024)
                       << " MB" << std::endl;
                buffer << "\t\tMax Work-group Total Size: " << dev->max_work_group_size()
                       << std::endl;

                std::vector<size_t> d
                    = dev->get_info<std::vector<size_t>>(CL_DEVICE_MAX_WORK_ITEM_SIZES);
                buffer << "\t\tMax Work-group Dims: (";
                for (std::vector<size_t>::iterator st = d.begin(); st != d.end(); st++)
                    buffer << *st << " ";
                buffer << "\x08)" << std::endl;

                buffer << "\t-------------------------" << std::endl;
            }

            buffer << "\n-------------------------\n";
        }
    } catch (opencl_error err) {
        buffer << "OpenCL Error: " << err.what() << " returned " << err.error_string() << std::endl;
    }
    return buffer.str();
}
}  // namespace ComputeEnvironment