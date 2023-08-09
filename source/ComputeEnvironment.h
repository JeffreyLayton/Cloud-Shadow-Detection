#pragma once
#include <string>

#include <boost/compute/cl.hpp>
#include <boost/compute/core.hpp>

namespace ComputeEnvironment {
extern boost::compute::context Context;
extern boost::compute::command_queue CommandQueue;

void InitMainContext();

std::string PlatformAndDeviceInfo();
}  // namespace ComputeEnvironment
