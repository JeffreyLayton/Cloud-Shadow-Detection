import os

from conan import ConanFile
from conan.tools.cmake import CMake
from conan.tools.files import copy

deps = {}

class Recipe(ConanFile):

    name = "Cloud-Shadow-Detection"
    version = "1.0.0"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports = "version.txt", "dependencies.json"
    exports_sources = "version.txt", "CMakeLists.txt", "CMakePresets.json", "cmake/*", "include/*", "source/*"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("lyra/1.6.1")
        self.requires("tomlplusplus/3.3.0")
        self.requires("eigen/3.4.0")
        self.requires("fmt/10.0.0")
        self.requires("imgui/1.81")
        self.requires("nlohmann_json/3.11.2")
        self.requires("glad/0.1.36")
        self.requires("glm/cci.20230113")
        self.requires("glfw/3.3.8")
        self.requires("libtiff/4.5.0")
        self.requires("boost/1.82.0")
        self.requires("opencl-headers/2023.04.17")
        self.requires("opencl-icd-loader/2023.04.17")
        self.requires("opengl/system")

    def generate(self):
        copy(self, "*glfw*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "bindings"))
        copy(self, "*opengl3*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "bindings"))

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={"DISABLE_DOCTEST":"YES"})
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def configure(self):
        self.options["glad"].gl_profile = "core"
        self.options["glad"].gl_version = "4.6"

        self.options["boost"].header_only = False
        self.options["boost"].without_chrono = False
        self.options["boost"].without_system = False
        self.options["boost"].without_thread = False
        self.options["boost"].without_exception = False

        self.options["boost"].without_test = True
        self.options["boost"].without_coroutine = True
        self.options["boost"].without_graph = True
        self.options["boost"].without_wave = True
        self.options["boost"].without_graph_parallel = True
        self.options["boost"].without_iostreams = True
        self.options["boost"].without_locale = True
        self.options["boost"].without_log = True
        self.options["boost"].without_math = True
        self.options["boost"].without_mpi = True
        self.options["boost"].without_program_options = True
        self.options["boost"].without_random = True
        self.options["boost"].without_regex = True
        self.options["boost"].without_serialization = True
        self.options["boost"].without_timer = True
        self.options["boost"].without_type_erasure = True


    def package_info(self):
        self.cpp_info.libs = ["LIBRARY_NAME"]
