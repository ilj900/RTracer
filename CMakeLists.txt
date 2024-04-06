cmake_minimum_required(VERSION 3.24)

project(RTracer)

set(CMAKE_CXX_STANDARD 17)

option(ENABLE_GPU_TIMINGS "Enables GPU timings for profiling" ON)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

add_subdirectory(3rdparty/glfw)
add_subdirectory(3rdparty/Catch2)
add_subdirectory(src/standalone)
add_subdirectory(src/test)
add_subdirectory(src/engine)
add_subdirectory(src/vulkan_helpers)
add_subdirectory(src/ECS)
add_subdirectory(src/utils)
add_subdirectory(src/math)
add_subdirectory(src/shaders)
add_subdirectory(3rdparty/tinyexr)
add_subdirectory(3rdparty/tiny_obj_loader)
add_subdirectory(3rdparty/stb_image)
add_subdirectory(3rdparty)