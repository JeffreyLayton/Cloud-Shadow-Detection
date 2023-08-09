cmake_minimum_required(VERSION 3.14)

# Every file should be used
file(
    GLOB SOURCES 
    ${CMAKE_SOURCE_DIR}/source/*.cpp 
    ${CMAKE_SOURCE_DIR}/source/boilerplate/*.cpp
    ${CMAKE_SOURCE_DIR}/bindings/*.cpp
)

add_executable(Cloud-Shadow-Detection_exe main-Cloud-Shadow-Detection.cpp ${SOURCES})
add_executable(Cloud-Shadow-Detection::exe ALIAS Cloud-Shadow-Detection_exe)
set_property(TARGET Cloud-Shadow-Detection_exe PROPERTY OUTPUT_NAME Cloud-Shadow-Detection)
target_compile_features(Cloud-Shadow-Detection_exe PRIVATE cxx_std_20)
target_compile_definitions(Cloud-Shadow-Detection_exe PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_include_directories(
    Cloud-Shadow-Detection_exe PRIVATE
    ${CMAKE_SOURCE_DIR}/source/boilerplate
    ${CMAKE_SOURCE_DIR}/source
    ${CMAKE_SOURCE_DIR}/bindings
)

target_link_libraries(
    Cloud-Shadow-Detection_exe PRIVATE 
    bfg::lyra
    tomlplusplus::tomlplusplus
    Eigen3::Eigen
    fmt::fmt
    imgui::imgui
    nlohmann_json::nlohmann_json
    glad::glad
    glm::glm
    glfw
    opengl::opengl
    TIFF::TIFF
    Boost::headers
    Boost::boost
    OpenCL::Headers
    OpenCL::OpenCL
)

add_custom_command(
    TARGET Cloud-Shadow-Detection_exe POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_directory  
    ${CMAKE_SOURCE_DIR}/source/shaders          
    $<TARGET_FILE_DIR:Cloud-Shadow-Detection_exe>/shaders 
)


