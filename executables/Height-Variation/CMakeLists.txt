cmake_minimum_required(VERSION 3.14)

# To reduce build, only use the limited files
file(
    GLOB SOURCES 
    ${CMAKE_SOURCE_DIR}/source/types.cpp 
    ${CMAKE_SOURCE_DIR}/source/Functions.cpp 
    ${CMAKE_SOURCE_DIR}/source/Imageio.cpp 
    ${CMAKE_SOURCE_DIR}/source/ImageOperations.cpp 
    ${CMAKE_SOURCE_DIR}/source/VectorGridOperations.cpp 
)

add_executable(Height-Variation_exe main-Height-Variation.cpp ${SOURCES})
add_executable(Height-Variation::exe ALIAS Height-Variation_exe)
set_property(TARGET Height-Variation_exe PROPERTY OUTPUT_NAME Height-Variation)
target_compile_features(Height-Variation_exe PRIVATE cxx_std_20)

target_include_directories(
    Height-Variation_exe PRIVATE
    ${CMAKE_SOURCE_DIR}/source/boilerplate
    ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(
    Height-Variation_exe PRIVATE 
    bfg::lyra
    tomlplusplus::tomlplusplus
    Eigen3::Eigen
    fmt::fmt
    nlohmann_json::nlohmann_json
    glm::glm
    TIFF::TIFF
)

add_custom_command(
    TARGET Height-Variation_exe POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_directory  
    ${CMAKE_SOURCE_DIR}/source/shaders          
    $<TARGET_FILE_DIR:Height-Variation_exe>/shaders 
)

