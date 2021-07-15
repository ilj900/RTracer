cmake_minimum_required(VERSION 3.13)

project(RTracer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(glfw)

include_directories(ECS
                    vulkan
                    glfw/include
                    ${Vulkan}/Include)

set(SOURCE  src/main.cpp
            vulkan/instance.cpp
            vulkan/physical_device.cpp
            vulkan/vulkan_helpers.cpp
            vulkan/director.cpp
            vulkan/logical_device.cpp)

set(INCLUDE ECS/entity.h
            vulkan/vulkan_helpers.h
            vulkan/instance.h
            vulkan/physical_device.h
            vulkan/director.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})
target_link_libraries(RTracer glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)