cmake_minimum_required(VERSION 3.14)

project(
    Cloud-Shadow-Detection
    VERSION 1.0.0
    DESCRIPTION "This software is a prototype implementation of our cloud shadow detection system. It will read the relevent input files from a source directory, compute the resulting outputs, and run the custom GUI to inspect results and/or output the results to the output directory."
    HOMEPAGE_URL "https://github.com/JeffreyLayton/Cloud-Shadow-Detection"
    LANGUAGES CXX
)

find_package(lyra REQUIRED)
find_package(tomlplusplus REQUIRED)
find_package(Eigen3 REQUIRED CONFIG)
find_package(fmt REQUIRED)
find_package(imgui REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(TIFF REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenCLHeaders REQUIRED)
find_package(OpenCLICDLoader REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(executables/Cloud-Shadow-Detection)
add_subdirectory(executables/Height-Variation)