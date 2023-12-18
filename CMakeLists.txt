cmake_minimum_required(VERSION 3.24)

project(RTracer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

add_subdirectory(src/standalone)
add_subdirectory(src/engine)
add_subdirectory(src/vulkan_helpers)
add_subdirectory(src/ECS)
add_subdirectory(src/math)
add_subdirectory(src/shaders)
add_subdirectory(src/utils)
add_subdirectory(3rdparty)